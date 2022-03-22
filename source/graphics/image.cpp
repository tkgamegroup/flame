#include "../foundation/bitmap.h"
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "extension.h"

#include <gli/gli.hpp>

namespace flame
{
	namespace graphics
	{
		std::vector<std::unique_ptr<ImageT>> loaded_images;
		std::vector<std::unique_ptr<SamplerT>> samplers;

		void ImagePrivate::initialize()
		{
			pixel_size = get_pixel_size(format);

			if (n_levels == 0)
				n_levels = 16;

			data_size = 0;
			auto s = size;
			for (auto i = 0; i < n_levels; i++)
			{
				auto& l = levels.emplace_back();
				l.size = s;
				l.pitch = image_pitch(pixel_size * s.x);
				l.data_size = s.y * l.pitch;
				data_size += l.data_size * n_layers;
				l.layers.resize(n_layers);

				s.x /= 2;
				s.y /= 2;
				if (n_levels == 0 && s.x == 0 && s.y == 0)
				{
					n_levels = i + 1;
					break;
				}
				if (s.x == 0) s.x = 1;
				if (s.y == 0) s.y = 1;
			}
		}

		ImagePrivate::~ImagePrivate()
		{
			if (app_exiting) return;

			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
				unregister_backend_object(vk_image);
			}
		}

		void ImagePrivate::get_data(uint level, uint layer)
		{
			auto& lv = levels[level];
			auto& ly = lv.layers [layer];
			if (!ly.data)
			{
				StagingBuffer sb(lv.data_size);
				{
					InstanceCB cb;
					BufferImageCopy cpy;
					cpy.img_ext = lv.size;
					cpy.img_sub = { level, 1, layer, 1 };
					cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferSrc);
					cb->copy_image_to_buffer(this, sb.get(), { &cpy, 1 });
					cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly);
				}
				ly.data.reset(new uchar[lv.data_size]);
				memcpy(ly.data.get(), sb->mapped, lv.data_size);
			}
		}

		vec4 ImagePrivate::get_pixel(int x, int y, uint level, uint layer)
		{
			auto& lv = levels[level];
			auto& ly = lv.layers[layer];

			x = clamp(x, 0, (int)lv.size.x - 1);
			y = clamp(y, 0, (int)lv.size.y - 1);

			auto pixel = ly.data.get() + lv.pitch * y + pixel_size * x;
			switch (format)
			{
			case Format_R8_UNORM:
				return vec4(pixel[0] / 255.f, 0.f, 0.f, 0.f);
			case Format_R8G8B8A8_UNORM:
				return vec4(pixel[0] / 255.f, pixel[1] / 255.f, pixel[2] / 255.f, pixel[3] / 255.f);
			case Format_R16G16B16A16_SFLOAT:
				return vec4(unpackHalf1x16(((ushort*)pixel)[0]), unpackHalf1x16(((ushort*)pixel)[1]),
					unpackHalf1x16(((ushort*)pixel)[2]), unpackHalf1x16(((ushort*)pixel)[3]));
			default:
				assert(0);
			}
		}

		void ImagePrivate::set_pixel(int x, int y, uint level, uint layer, const vec4& v)
		{
			auto& lv = levels[level];
			auto& ly = lv.layers[layer];

			x = clamp(x, 0, (int)lv.size.x - 1);
			y = clamp(y, 0, (int)lv.size.y - 1);

			auto pixel = ly.data.get() + lv.pitch * y + pixel_size * x;
			switch (format)
			{
			case Format_R8_UNORM:
				pixel[0] = int(clamp(v[0], 0.f, 1.f) * 255.f);
				break;
			case Format_R8G8B8A8_UNORM:
				pixel[0] = int(clamp(v[0], 0.f, 1.f) * 255.f);
				pixel[1] = int(clamp(v[1], 0.f, 1.f) * 255.f);
				pixel[2] = int(clamp(v[2], 0.f, 1.f) * 255.f);
				pixel[3] = int(clamp(v[3], 0.f, 1.f) * 255.f);
				break;
			default:
				assert(0);
			}
		}

		ImageViewPtr ImagePrivate::get_view(const ImageSub& sub, const ImageSwizzle& swizzle)
		{
			uint64 key;
			{
				uint hi;
				uint lo;
				hi = (sub.base_level & 0xff) << 24;
				hi |= (sub.level_count & 0xff) << 16;
				hi |= (sub.base_layer & 0xff) << 8;
				hi |= sub.layer_count & 0xff;
				lo = (swizzle.r & 0xff) << 24;
				lo |= (swizzle.g & 0xff) << 16;
				lo |= (swizzle.b & 0xff) << 8;
				lo |= swizzle.a & 0xff;
				key = (((uint64)hi) << 32) | ((uint64)lo);
			}

			auto it = views.find(key);
			if (it != views.end())
				return it->second.get();

			auto iv = new ImageViewPrivate;
			iv->image = this;
			iv->sub = sub;
			iv->swizzle = swizzle;

			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = to_backend(swizzle.r);
			info.components.g = to_backend(swizzle.g);
			info.components.b = to_backend(swizzle.b);
			info.components.a = to_backend(swizzle.a);
			info.image = vk_image;
			if (is_cube && sub.base_layer == 0 && sub.layer_count == 6)
				info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			else if (sub.layer_count > 1)
				info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			else
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = to_backend(format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(format));
			info.subresourceRange.baseMipLevel = sub.base_level;
			info.subresourceRange.levelCount = sub.level_count;
			info.subresourceRange.baseArrayLayer = sub.base_layer;
			info.subresourceRange.layerCount = sub.layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &iv->vk_image_view));
			register_backend_object(iv->vk_image_view, th<decltype(*iv)>(), iv);

			views.emplace(key, iv);
			return iv;
		}

		static DescriptorSetLayoutPrivate* simple_dsl = nullptr;

		DescriptorSetPtr ImagePrivate::get_shader_read_src(uint base_level, uint base_layer, SamplerPtr sp)
		{
			if (!sp)
				sp = SamplerPrivate::get(FilterLinear, FilterLinear, false, AddressClampToEdge);

			auto key = (base_level & 0xff) << 24;
			key |= (base_layer & 0xff) << 16;
			key |= (uint)sp & 0xffff;

			auto it = read_dss.find(key);
			if (it != read_dss.end())
				return it->second.get();

			if (!simple_dsl)
			{
				DescriptorBinding b;
				b.type = DescriptorSampledImage;
				simple_dsl = DescriptorSetLayout::create({ &b, 1 });
			}

			auto ds = DescriptorSet::create(nullptr, simple_dsl);
			ds->set_image(0, 0, get_view({ base_level, 1, base_layer, 1 }), sp);
			ds->update();
			read_dss.emplace(key, ds);
			return ds;
		}

		static std::vector<RenderpassPrivate*> simple_rps;

		FramebufferPtr ImagePrivate::get_shader_write_dst(uint base_level, uint base_layer, AttachmentLoadOp load_op)
		{
			auto key = (base_level & 0xff) << 24;
			key |= (base_layer & 0xff) << 16;
			key |= (uint)load_op & 0xffff;

			auto it = write_fbs.find(key);
			if (it != write_fbs.end())
				return it->second.get();

			RenderpassPrivate* rp = nullptr;
			for (auto& r : simple_rps)
			{
				auto& att = r->attachments[0];
				if (att.format == format && att.load_op == load_op && att.sample_count == sample_count)
				{
					rp = r;
					break;
				}
			}
			if (!rp)
			{
				RenderpassInfo info;
				auto& att = info.attachments.emplace_back();
				att.format = format;
				att.load_op = load_op;
				att.sample_count = sample_count;
				switch (load_op)
				{
				case AttachmentLoadLoad:
					att.initia_layout = ImageLayoutAttachment;
					break;
				}
				auto& sp = info.subpasses.emplace_back();
				if (format >= Format_Color_Begin && format <= Format_Color_End)
					sp.color_attachments.push_back(0);
				else
					sp.depth_attachment = 0;
				rp = Renderpass::create(info);
				simple_rps.push_back(rp);
			}

			auto fb = Framebuffer::create(rp, get_view({ base_level, 1, base_layer, 1 }));
			write_fbs.emplace(key, fb);
			return fb;
		}

		void ImagePrivate::change_layout(ImageLayout dst_layout)
		{
			InstanceCB cb;

			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, dst_layout);
		}

		void ImagePrivate::clear(const vec4& color, ImageLayout dst_layout)
		{
			InstanceCB cb;

			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, ImageLayoutTransferDst);
			cb->clear_color_image(this, { 0, n_levels, 0, n_layers }, color);
			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, dst_layout);
		}

		vec4 ImagePrivate::linear_sample(const vec2& uv, uint level, uint layer)
		{
			get_data(level, layer);

			auto coord = uv * vec2(levels[level].size) - 0.5f;
			auto coordi = ivec2(floor(coord));
			auto coordf = coord - vec2(coordi);

			return mix(
				mix(get_pixel(coordi.x, coordi.y, level, layer), get_pixel(coordi.x + 1, coordi.y, level, layer), coordf.x),
				mix(get_pixel(coordi.x, coordi.y + 1, level, layer), get_pixel(coordi.x + 1, coordi.y + 1, level, layer), coordf.x),
				coordf.y);
		}

		void ImagePrivate::save(const std::filesystem::path& filename)
		{
			assert(usage & ImageUsageTransferSrc);

			auto ext = filename.extension();
			if (ext == L".dds" || ext == L".ktx")
			{
				auto gli_fmt = gli::FORMAT_UNDEFINED;
				switch (format)
				{
				case Format_R8G8B8A8_UNORM:
					gli_fmt = gli::FORMAT_RGBA8_UNORM_PACK8;
					break;
				case Format_R16G16B16A16_SFLOAT:
					gli_fmt = gli::FORMAT_RGBA16_SFLOAT_PACK16;
					break;
				}
				assert(gli_fmt != gli::FORMAT_UNDEFINED);

				auto gli_texture = gli::texture(gli::TARGET_2D, gli_fmt, ivec3(size, 1), n_layers, 1, n_levels);

				StagingBuffer sb(data_size, nullptr);
				std::vector<std::tuple<void*, void*, uint>> gli_cpies;
				{
					InstanceCB cb;
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)sb->mapped;
					auto offset = 0;
					for (auto i = 0; i < n_layers; i++)
					{
						for (auto j = 0; j < n_levels; j++)
						{
							auto size = (uint)gli_texture.size(j);

							gli_cpies.emplace_back(gli_texture.data(i, 0, j), dst + offset, size);

							BufferImageCopy cpy;
							cpy.buf_off = offset;
							auto ext = gli_texture.extent(j);
							cpy.img_ext = uvec2(ext.x, ext.y);
							cpy.img_sub.base_level = j;
							cpy.img_sub.base_layer = i;
							cpies.push_back(cpy);

							offset += size;
						}
					}
					cb->image_barrier(this, { 0, n_levels, 0, n_layers }, ImageLayoutTransferSrc);
					cb->copy_image_to_buffer(this, sb.get(), cpies);
					cb->image_barrier(this, { 0, n_levels, 0, n_layers }, ImageLayoutShaderReadOnly);
				}
				for (auto& c : gli_cpies)
					memcpy(std::get<0>(c), std::get<1>(c), std::get<2>(c));

				gli::save(gli_texture, filename.string());
			}
			else
			{

			}
		}

		float get_image_alphatest_coverage(ImagePtr img, uint level, float ref, float scale)
		{
			assert(img->format == Format_R8G8B8A8_UNORM || img->format == Format_R8_UNORM);
			auto ch = img->format == Format_R8G8B8A8_UNORM ? 3 : 0;

			img->get_data(level, 0);

			auto coverage = 0.f;
			auto size = img->levels[level].size;
			for (auto y = 0; y < size.y; y++)
			{
				for (auto x = 0; x < size.x; x++)
				{
					if (img->get_pixel(x, y, level, 0)[ch] * scale > ref)
						coverage += 1.f;
				}
			}

			return coverage / float(size.x * size.y);
		}

		void scale_image_alphatest_coverage(ImagePtr img, uint level, float desired, float ref)
		{
			assert(img->format == Format_R8G8B8A8_UNORM || img->format == Format_R8_UNORM);
			auto ch = img->format == Format_R8G8B8A8_UNORM ? 3 : 0;

			auto min_alpha_scale = 0.f;
			auto max_alpha_scale = 4.f;
			auto alpha_scale = 1.f;
			auto best_alpha_scale = 1.f;
			auto best_error = std::numeric_limits<float>::max();

			for (int i = 0; i < 10; i++)
			{
				auto current_coverage = get_image_alphatest_coverage(img, level, ref, alpha_scale);

				auto error = abs(current_coverage - desired);
				if (error < best_error)
				{
					best_error = error;
					best_alpha_scale = alpha_scale;
				}

				if (current_coverage < desired)
					min_alpha_scale = alpha_scale;
				else if (current_coverage > desired)
					max_alpha_scale = alpha_scale;
				else
					break;

				alpha_scale = (min_alpha_scale + max_alpha_scale) * 0.5f;
			}

			auto& lv = img->levels[level];
			auto& ly = lv.layers[0];
			img->get_data(level, 0);
			for (auto y = 0; y < lv.size.y; y++)
			{
				for (auto x = 0; x < lv.size.x; x++)
				{
					auto pos = ivec2(x, y);
					auto v = img->get_pixel(x, y, level, 0);
					v[ch] *= best_alpha_scale;
					img->set_pixel(x, y, level, 0, v);
				}
			}

			{
				StagingBuffer sb(lv.data_size, ly.data.get());
				InstanceCB cb;
				BufferImageCopy cpy;
				cpy.img_ext = lv.size;
				cpy.img_sub.base_level = level;
				cb->image_barrier(img, cpy.img_sub, ImageLayoutTransferDst);
				cb->copy_buffer_to_image(sb.get(), img, { &cpy, 1 });
				cb->image_barrier(img, cpy.img_sub, ImageLayoutShaderReadOnly);
			}
		}

		struct ImageCreate : Image::Create
		{
			ImagePtr operator()(Format format, const uvec2& size, ImageUsageFlags usage, uint levels, uint layers, SampleCount sample_count, bool is_cube) override
			{
				auto ret = new ImagePrivate;
				ret->format = format;
				ret->n_levels = levels;
				ret->n_layers = layers;
				ret->sample_count = sample_count;
				ret->usage = usage;
				ret->is_cube = is_cube;
				ret->size = size;
				ret->initialize();

				VkImageCreateInfo imageInfo;
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
				imageInfo.pNext = nullptr;
				imageInfo.imageType = VK_IMAGE_TYPE_2D;
				imageInfo.format = to_backend(format);
				imageInfo.extent.width = size.x;
				imageInfo.extent.height = size.y;
				imageInfo.extent.depth = 1;
				imageInfo.mipLevels = levels;
				imageInfo.arrayLayers = layers;
				imageInfo.samples = to_backend(sample_count);
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.usage = get_backend_image_usage_flags(usage, format, sample_count);
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.queueFamilyIndexCount = 0;
				imageInfo.pQueueFamilyIndices = nullptr;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				chk_res(vkCreateImage(device->vk_device, &imageInfo, nullptr, &ret->vk_image));

				VkMemoryRequirements memRequirements;
				vkGetImageMemoryRequirements(device->vk_device, ret->vk_image, &memRequirements);

				VkMemoryAllocateInfo allocInfo;
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.pNext = nullptr;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = device->find_memory_type(memRequirements.memoryTypeBits, MemoryPropertyDevice);

				chk_res(vkAllocateMemory(device->vk_device, &allocInfo, nullptr, &ret->vk_memory));
				chk_res(vkBindImageMemory(device->vk_device, ret->vk_image, ret->vk_memory, 0));
				register_backend_object(ret->vk_image, th<decltype(*ret)>(), ret);

				return ret;
			}

			ImagePtr operator()(Format format, const uvec2& size, void* data) override
			{
				auto ret = Image::create(format, size, ImageUsageSampled | ImageUsageTransferDst);

				StagingBuffer stag(image_pitch(get_pixel_size(format) * size.x) * size.y, data);
				InstanceCB cb;
				cb->image_barrier(ret, {}, ImageLayoutTransferDst);
				BufferImageCopy cpy;
				cpy.img_ext = size;
				cb->copy_buffer_to_image(stag.get(), ret, { &cpy, 1 });
				cb->image_barrier(ret, {}, ImageLayoutShaderReadOnly);

				return ret;
			}
		}Image_create;
		Image::Create& Image::create = Image_create;

		struct ImageGet : Image::Get
		{
			ImagePtr operator()(const std::filesystem::path& _filename, bool srgb, const MipmapOption& mipmap_option) override
			{
				auto filename = Path::get(_filename);

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find image: %s\n", _filename.c_str());
					return nullptr;
				}

				ImagePtr ret = nullptr;

				auto ext = filename.extension();
				if (ext == L".ktx" || ext == L".dds")
				{
					auto is_cube = false;

					auto gli_texture = gli::load(filename.string());
					if (gli_texture.empty())
					{
						wprintf(L"cannot load image: %s\n", _filename.c_str());
						return nullptr;
					}

					auto ext = gli_texture.extent();
					auto levels = (uint)gli_texture.levels();
					auto layers = (uint)gli_texture.layers();
					auto faces = (uint)gli_texture.faces();
					if (faces > 1)
					{
						assert(layers == 1);
						layers = faces;
					}
					if (layers == 6)
						is_cube = true;

					Format format = Format_Undefined;
					switch (gli_texture.format())
					{
					case gli::FORMAT_RGBA8_UNORM_PACK8:
						format = Format_R8G8B8A8_UNORM;
						break;
					case gli::FORMAT_RGBA16_SFLOAT_PACK16:
						format = Format_R16G16B16A16_SFLOAT;
						break;
					case gli::FORMAT_RGBA32_SFLOAT_PACK32:
						format = Format_R32G32B32A32_SFLOAT;
						break;
					}
					assert(format != Format_Undefined);

					ret = Image::create(format, ext, ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc,
						levels, layers, SampleCount_1, is_cube);

					StagingBuffer sb(ret->data_size, nullptr);
					InstanceCB cb;
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)sb->mapped;
					auto offset = 0;
					for (auto i = 0; i < layers; i++)
					{
						for (auto j = 0; j < levels; j++)
						{
							auto size = gli_texture.size(j);
							auto ext = gli_texture.extent(j);
							void* data;
							if (faces > 1)
								data = gli_texture.data(0, i, j);
							else
								data = gli_texture.data(i, 0, j);
							memcpy(dst + offset, data, size);

							BufferImageCopy cpy;
							cpy.buf_off = offset;
							cpy.img_ext = ext;
							cpy.img_sub.base_level = j;
							cpy.img_sub.base_layer = i;
							cpies.push_back(cpy);

							offset += size;
						}
					}
					cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutTransferDst);
					cb->copy_buffer_to_image(sb.get(), ret, cpies);
					cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutShaderReadOnly);
				}
				else
				{
					std::unique_ptr<Bitmap> bmp(Bitmap::create(filename, 4));
					if (!bmp)
					{
						wprintf(L"cannot load image: %s\n", _filename.c_str());
						return nullptr;
					}

					if (srgb)			bmp->srgb_to_linear();
					if (bmp->chs == 3)	bmp->change_format(4);

					ret = Image::create(get_image_format(bmp->chs, bmp->bpp), bmp->size,
						ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc, mipmap_option.auto_gen ? 0 : 1);

					{
						StagingBuffer sb(bmp->data_size, bmp->data);
						InstanceCB cb;
						BufferImageCopy cpy;
						cpy.img_ext = ret->size;
						cb->image_barrier(ret, {}, ImageLayoutTransferDst);
						cb->copy_buffer_to_image(sb.get(), ret, { &cpy, 1 });
						if (mipmap_option.auto_gen)
						{
							for (auto i = 1U; i < ret->n_levels; i++)
							{
								cb->image_barrier(ret, { i - 1, 1, 0, 1 }, ImageLayoutTransferSrc);
								ImageBlit blit;
								blit.src_sub.base_level = i - 1;
								blit.src_range = ivec4(0, 0, ret->levels[i - 1].size);
								blit.dst_sub.base_level = i;
								blit.dst_range = ivec4(0, 0, ret->levels[i].size);
								cb->blit_image(ret, ret, { &blit, 1 }, FilterLinear);
							}
						}
						cb->image_barrier(ret, { 0, ret->n_levels, 0, 1 }, ImageLayoutShaderReadOnly);
					}

					if (mipmap_option.auto_gen && mipmap_option.alpha_test > 0.f)
					{
						auto coverage = get_image_alphatest_coverage(ret, 0, mipmap_option.alpha_test, 1.f);
						for (auto i = 1; i < ret->n_levels; i++)
							scale_image_alphatest_coverage(ret, i, coverage, mipmap_option.alpha_test);
					}

					ret->filename = filename;
					ret->srgb = srgb;
				}

				ret->ref = 1;
				loaded_images.emplace_back(ret);
				return ret;
			}
		}Image_get;
		Image::Get& Image::get = Image_get;

		struct ImageRelease : Image::Release
		{
			void operator()(ImagePtr image) override
			{
				if (image->ref == 1)
				{
					graphics::Queue::get()->wait_idle();
					std::erase_if(loaded_images, [&](const auto& i) {
						return i.get() == image;
					});
				}
				else
					image->ref--;
			}
		}Image_release;
		Image::Release& Image::release = Image_release;

		ImagePtr ImagePrivate::create(DevicePtr device, Format format, const uvec2& size, VkImage native)
		{
			auto ret = new ImagePrivate;

			ret->format = format;
			ret->size = size;
			ret->initialize();
			ret->vk_image = native;
			register_backend_object(ret->vk_image, th<decltype(*ret)>(), ret);

			return ret;
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			if (app_exiting) return;

			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
			unregister_backend_object(vk_image_view);
		}

		SamplerPrivate::~SamplerPrivate()
		{
			if (app_exiting) return;

			vkDestroySampler(device->vk_device, vk_sampler, nullptr);
			unregister_backend_object(vk_sampler);
		}

		struct SamplerGet : Sampler::Get
		{
			SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode) override
			{
				for (auto& s : samplers)
				{
					if (s->mag_filter == mag_filter && s->min_filter == min_filter && s->linear_mipmap == linear_mipmap && s->address_mode == address_mode)
						return s.get();
				}

				auto ret = new SamplerPrivate;
				ret->mag_filter = mag_filter;
				ret->min_filter = min_filter;
				ret->linear_mipmap = linear_mipmap;
				ret->address_mode = address_mode;

				VkSamplerCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				info.magFilter = to_backend(mag_filter);
				info.minFilter = to_backend(min_filter);
				info.mipmapMode = linear_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
				info.addressModeU = info.addressModeV = info.addressModeW = to_backend(address_mode);
				info.maxAnisotropy = 1.f;
				info.maxLod = VK_LOD_CLAMP_NONE;
				info.compareOp = VK_COMPARE_OP_ALWAYS;
				info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

				chk_res(vkCreateSampler(device->vk_device, &info, nullptr, &ret->vk_sampler));
				register_backend_object(ret->vk_sampler, th<decltype(*ret)>(), ret);

				samplers.emplace_back(ret);
				return ret;
			}
		}Sampler_get;
		Sampler::Get& Sampler::get = Sampler_get;

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			Image::release(image);
		}

		static std::vector<std::pair<std::filesystem::path, std::unique_ptr<ImageAtlasPrivate>>> loaded_atlas;

		struct ImageAtlasGet : ImageAtlas::Get
		{
			ImageAtlasPtr operator()(const std::filesystem::path& filename) override
			{
				for (auto& a : loaded_atlas)
				{
					if (a.first == filename)
						return a.second.get();
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find atlas: %s\n", filename);
					return nullptr;
				}

				auto ret = new ImageAtlasPrivate;

				auto png_filename = filename;
				png_filename.replace_extension(L".png");
				ret->image = Image::get(png_filename, false);

				auto size = (vec2)ret->image->size;

				for (auto& e : parse_ini_file(filename).get_section_entries(""))
				{
					auto& tile = ret->tiles.emplace_back();
					tile.index = ret->tiles.size();
					auto sp = SUS::split(e.value);
					tile.name = sp[0];
					auto v = s2t<4, uint>(sp[1]);
					tile.pos = ivec2(v.x, v.y);
					tile.size = ivec2(v.z, v.w);
					tile.uv.x = tile.pos.x / size.x;
					tile.uv.y = tile.pos.y / size.y;
					tile.uv.z = (tile.pos.x + tile.size.x) / size.x;
					tile.uv.w = (tile.pos.y + tile.size.y) / size.y;
				}

				loaded_atlas.emplace_back(filename, ret);
				return ret;
			}
		}ImageAtlas_get;
		ImageAtlas::Get& ImageAtlas::get = ImageAtlas_get;
	}
}

