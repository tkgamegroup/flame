#include "../foundation/bitmap.h"
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "extension.h"

#include <gli/gli.hpp>
#ifdef USE_NVTT
#include <nvtt/nvtt.h>
#include <nvtt/nvtt_lowlevel.h>
#endif

namespace flame
{
	namespace graphics
	{
		std::vector<ImagePtr> images;
		std::vector<LoadedImage> loaded_images;
		std::vector<std::unique_ptr<SamplerT>> samplers;

		ImagePrivate::ImagePrivate()
		{
			images.push_back(this);
		}

		ImagePrivate::~ImagePrivate()
		{
			if (app_exiting) return;

			std::erase_if(images, [&](const auto& i) {
				return i == this;
			});

			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
			}
			unregister_object(vk_image);
			unregister_object(vk_memory);
		}

		void ImagePrivate::initialize()
		{
			n_channels = get_num_channels(format);
			pixel_size = get_pixel_size(format);

			if (n_levels == 0)
			{
				if (extent.z == 1)
					n_levels = 16;
				else
					n_levels = 1;
			}

			auto ext = extent;
			for (auto i = 0; i < n_levels; i++)
			{
				auto& l = levels.emplace_back();
				l.extent = ext;
				l.pitch = image_pitch(pixel_size * ext.x);
				l.data_size = ext.z * ext.y * l.pitch;
				l.layers.resize(n_layers);

				ext.x /= 2;
				ext.y /= 2;
				if (ext.x == 0 && ext.y == 0)
				{
					n_levels = i + 1;
					break;
				}
				if (ext.x == 0) ext.x = 1;
				if (ext.y == 0) ext.y = 1;
			}

			data_size = 0;
			for (auto& l : levels)
				data_size += l.data_size * n_layers;
		}

		ImageLayout ImagePrivate::get_layout(const ImageSub& sub)
		{
			ImageLayout ret = (ImageLayout)0xffffffff;
			for (auto i = 0; i < sub.level_count; i++)
			{
				auto& lv = levels[sub.base_level + i];
				for (auto j = 0; j < sub.layer_count; j++)
				{
					auto layout = lv.layers[sub.base_layer + j].layout;
					if (ret == (ImageLayout)0xffffffff)
						ret = layout;
					else
						assert(ret == layout);
				}
			}
			return ret;
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
				lo = swizzle.r << 24;
				lo |= swizzle.g << 16;
				lo |= swizzle.b << 8;
				lo |= swizzle.a;
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
			else if (extent.z > 1)
				info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			else
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = to_backend(format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(format));
			info.subresourceRange.baseMipLevel = sub.base_level;
			info.subresourceRange.levelCount = sub.level_count;
			info.subresourceRange.baseArrayLayer = sub.base_layer;
			info.subresourceRange.layerCount = sub.layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &iv->vk_image_view));
			register_object(iv->vk_image_view, "Image View", iv);

			views.emplace(key, iv);
			return iv;
		}

		static DescriptorSetLayoutPrivate* simple_dsl = nullptr;

		DescriptorSetPtr ImagePrivate::get_shader_read_src(uint base_level, uint base_layer, SamplerPtr sp, const ImageSwizzle& swizzle)
		{
			if (!sp)
				sp = SamplerPrivate::get(FilterLinear, FilterLinear, false, AddressClampToEdge);

			uint64 key;
			{
				uint hi;
				uint lo;
				hi = (base_level & 0xff) << 24;
				hi |= (base_layer & 0xff) << 16;
				hi |= swizzle.r << 12;
				hi |= swizzle.g << 8;
				hi |= swizzle.b << 4;
				hi |= swizzle.a;

				lo = (uint)sp;
				key = (((uint64)hi) << 32) | ((uint64)lo);
			}

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
			ds->set_image_i(0, 0, get_view({ base_level, 1, base_layer, 1 }, swizzle), sp);
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
			InstanceCommandBuffer cb;
			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, dst_layout);
			cb.excute();
		}

		void ImagePrivate::clear(const vec4& color, ImageLayout dst_layout)
		{
			InstanceCommandBuffer cb;
			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, ImageLayoutTransferDst);
			cb->clear_color_image(this, { 0, n_levels, 0, n_layers }, color);
			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, dst_layout);
			cb.excute();
		}

		void ImagePrivate::stage_surface_data(uint level, uint layer)
		{
			auto& lv = levels[level];
			auto& ly = lv.layers[layer];
			if (!ly.data)
			{
				StagingBuffer sb(lv.data_size);
				InstanceCommandBuffer cb;
				BufferImageCopy cpy;
				cpy.img_ext = uvec3(lv.extent, 1);
				cpy.img_sub = { level, 1, layer, 1 };
				cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferSrc);
				cb->copy_image_to_buffer(this, sb.get(), { &cpy, 1 });
				cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly);
				cb.excute();
				ly.data.reset(new uchar[lv.data_size]);
				memcpy(ly.data.get(), sb->mapped, lv.data_size);
			}
		}

		vec4 ImagePrivate::get_pixel(int x, int y, uint level, uint layer)
		{
			stage_surface_data(level, layer);

			auto& lv = levels[level];
			auto& ly = lv.layers[layer];

			x = clamp(x, 0, (int)lv.extent.x - 1);
			y = clamp(y, 0, (int)lv.extent.y - 1);

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

			x = clamp(x, 0, (int)lv.extent.x - 1);
			y = clamp(y, 0, (int)lv.extent.y - 1);

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

		void ImagePrivate::clear_staging_data()
		{
			for (auto level = 0; level < n_levels; level++)
			{
				auto& lv = levels[level];
				for (auto layer = 0; layer < n_layers; layer++)
				{
					auto& ly = lv.layers[layer];
					ly.data.reset();
				}
			}
		}

		vec4 ImagePrivate::linear_sample(const vec2& uv, uint level, uint layer)
		{
			auto coord = uv * vec2(levels[level].extent) - 0.5f;
			auto coordi = ivec2(floor(coord));
			auto coordf = coord - vec2(coordi);

			return mix(
				mix(get_pixel(coordi.x, coordi.y, level, layer), get_pixel(coordi.x + 1, coordi.y, level, layer), coordf.x),
				mix(get_pixel(coordi.x, coordi.y + 1, level, layer), get_pixel(coordi.x + 1, coordi.y + 1, level, layer), coordf.x),
				coordf.y);
		}

		void ImagePrivate::save(const std::filesystem::path& filename, bool compress)
		{
			assert(usage & ImageUsageTransferSrc);

			auto ext = filename.extension();
			if (ext == L".dds")
			{
				auto gli_target = gli::TARGET_2D;
				if (is_cube)
					gli_target = gli::TARGET_CUBE;
				else if (n_layers > 1)
					gli_target = gli::TARGET_2D_ARRAY;
				else if (extent.z > 1)
					gli_target = gli::TARGET_3D;


				if (compress)
				{
#ifdef USE_NVTT
					assert(n_levels == 1); assert(n_layers == 1);
					assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM);
					nvtt::RefImage img_in;
					img_in.width = extent.x;
					img_in.height = extent.y;
					img_in.depth = extent.z;
					img_in.num_channels = n_channels;
					stage_surface_data(0, 0); auto& ly = levels[0].layers[0];
					img_in.data = ly.data.get();
					nvtt::CPUInputBuffer input_buf(&img_in, nvtt::UINT8);
					ly.data.reset(nullptr);
					if (n_channels == 1) // bc4
					{
						auto data_size = input_buf.NumTiles() * 8;
						void* outbuf = malloc(data_size);
						nvtt_encode_bc4(input_buf, false, outbuf, true, false);
						auto gli_texture = gli::texture(gli_target, gli::FORMAT_R_ATI1N_UNORM_BLOCK8, ivec3(extent), n_layers, 1, n_levels);
						memcpy(gli_texture.data(0, 0, 0), outbuf, data_size);
						gli::save(gli_texture, filename.string());
						free(outbuf);
					}
					else // bc7
					{
						auto data_size = input_buf.NumTiles() * 16;
						void* outbuf = malloc(data_size);
						nvtt_encode_bc7(input_buf, false, true, outbuf, true, false);
						auto gli_texture = gli::texture(gli_target, gli::FORMAT_RGBA_BP_UNORM_BLOCK16, ivec3(extent), n_layers, 1, n_levels);
						memcpy(gli_texture.data(0, 0, 0), outbuf, data_size);
						gli::save(gli_texture, filename.string());
						free(outbuf);
					}
					return;
#endif
				}

				auto gli_fmt = gli::FORMAT_UNDEFINED;
				switch (format)
				{
				case Format_R8_UNORM:
					gli_fmt = gli::FORMAT_R8_UNORM_PACK8;
					break;
				case Format_R8G8B8A8_UNORM:
					gli_fmt = gli::FORMAT_RGBA8_UNORM_PACK8;
					break;
				case Format_R16G16B16A16_SFLOAT:
					gli_fmt = gli::FORMAT_RGBA16_SFLOAT_PACK16;
					break;
				}
				assert(gli_fmt != gli::FORMAT_UNDEFINED);

				auto gli_texture = gli::texture(gli_target, gli_fmt, ivec3(extent), n_layers, 1, n_levels);

				for (auto i = 0; i < n_levels; i++)
				{
					auto size = (uint)gli_texture.size(i);
					for (auto j = 0; j < n_layers; j++)
					{
						stage_surface_data(i, j);
						memcpy(gli_texture.data(j, 0, i), levels[i].layers[j].data.get(), size);
					}
				}
				gli::save(gli_texture, filename.string());
			}
			else
			{
				StagingBuffer sb(data_size, nullptr);
				InstanceCommandBuffer cb;
				cb->image_barrier(this, {}, graphics::ImageLayoutTransferSrc);
				BufferImageCopy cpy;
				cpy.img_ext = extent;
				cb->copy_image_to_buffer(this, sb.get(), { &cpy, 1 });
				cb->image_barrier(this, {}, graphics::ImageLayoutShaderReadOnly);
				cb.excute();

				int chs = 0;
				switch (format)
				{
				case Format_R8_UNORM: chs = 1; break;
				case Format_R8G8B8A8_UNORM: chs = 4; break;
				}

				auto bmp = Bitmap::create(extent, chs);
				memcpy(bmp->data, sb->mapped, sb->size);
				bmp->save(replace_ext(filename, L".png"));
				delete bmp;
			}
		}

		float get_image_alphatest_coverage(ImagePtr img, uint level, float ref, float scale)
		{
			assert(img->format == Format_R8G8B8A8_UNORM || img->format == Format_R8_UNORM);
			auto ch = img->format == Format_R8G8B8A8_UNORM ? 3 : 0;

			img->stage_surface_data(level, 0);

			auto coverage = 0.f;
			auto size = img->levels[level].extent;
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
			img->stage_surface_data(level, 0);
			for (auto y = 0; y < lv.extent.y; y++)
			{
				for (auto x = 0; x < lv.extent.x; x++)
				{
					auto pos = ivec2(x, y);
					auto v = img->get_pixel(x, y, level, 0);
					v[ch] *= best_alpha_scale;
					img->set_pixel(x, y, level, 0, v);
				}
			}

			StagingBuffer sb(lv.data_size, ly.data.get());
			InstanceCommandBuffer cb;
			BufferImageCopy cpy;
			cpy.img_ext = uvec3(lv.extent, 1);
			cpy.img_sub.base_level = level;
			cb->image_barrier(img, cpy.img_sub, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(sb.get(), img, { &cpy, 1 });
			cb->image_barrier(img, cpy.img_sub, ImageLayoutShaderReadOnly);
			cb.excute();
		}

		struct ImageCreate : Image::Create
		{
			ImagePtr operator()(Format format, const uvec3& extent, ImageUsageFlags usage, uint levels, uint layers, SampleCount sample_count, bool is_cube) override
			{
				auto ret = new ImagePrivate;
				ret->format = format;
				ret->n_levels = levels;
				ret->n_layers = layers;
				ret->sample_count = sample_count;
				ret->usage = usage;
				ret->is_cube = is_cube;
				ret->extent = extent;
				ret->initialize();

				auto image_type = VK_IMAGE_TYPE_2D;
				if (extent.z > 1)
					image_type = VK_IMAGE_TYPE_3D;

				VkImageCreateInfo imageInfo;
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
				imageInfo.pNext = nullptr;
				imageInfo.imageType = image_type;
				imageInfo.format = to_backend(format);
				imageInfo.extent.width = extent.x;
				imageInfo.extent.height = extent.y;
				imageInfo.extent.depth = extent.z;
				imageInfo.mipLevels = ret->n_levels;
				imageInfo.arrayLayers = ret->n_layers;
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
				register_object(ret->vk_image, "Image", ret);
				register_object(ret->vk_memory, "Image Memory", ret);

				return ret;
			}

			ImagePtr operator()(Format format, const uvec3& extent, void* data) override
			{
				auto ret = Image::create(format, extent, ImageUsageSampled | ImageUsageTransferDst);

				StagingBuffer stag(ret->data_size, data);
				InstanceCommandBuffer cb;
				cb->image_barrier(ret, {}, ImageLayoutTransferDst);
				BufferImageCopy cpy;
				cpy.img_ext = extent;
				cb->copy_buffer_to_image(stag.get(), ret, { &cpy, 1 });
				cb->image_barrier(ret, {}, ImageLayoutShaderReadOnly);
				cb.excute();

				return ret;
			}
		}Image_create;
		Image::Create& Image::create = Image_create;

		struct ImageGet : Image::Get
		{
			ImagePtr operator()(const std::filesystem::path& _filename, bool srgb, bool auto_mipmapping, float alpha_coverage, ImageUsageFlags additional_usage) override
			{
				auto filename = Path::get(_filename);

				for (auto& i : loaded_images)
				{
					if (i.v->filename == filename && i.srgb == srgb && i.auto_mipmapping == auto_mipmapping && i.alpha_coverage == alpha_coverage && i.additional_usage == additional_usage)
					{
						i.v->ref++;
						return i.v.get();
					}
				}

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

					auto gli_format = gli_texture.format();
					Format format = Format_Undefined;
					switch (gli_format)
					{
					case gli::FORMAT_R8_UNORM_PACK8:
						format = Format_R8_UNORM;
						break;
					case gli::FORMAT_RGBA8_UNORM_PACK8:
						format = Format_R8G8B8A8_UNORM;
						break;
					case gli::FORMAT_RGBA16_SFLOAT_PACK16:
						format = Format_R16G16B16A16_SFLOAT;
						break;
					case gli::FORMAT_RGBA32_SFLOAT_PACK32:
						format = Format_R32G32B32A32_SFLOAT;
						break;
					case gli::FORMAT_R_ATI1N_UNORM_BLOCK8:
						format = Format_BC4_UNORM;
						break;
					case gli::FORMAT_RGBA_BP_UNORM_BLOCK16:
						format = Format_BC7_UNORM;
						break;
					}
					assert(format != Format_Undefined);

					ret = Image::create(format, ext, ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc | additional_usage,
						levels, layers, SampleCount_1, is_cube);

					StagingBuffer sb(ret->data_size, nullptr);
					InstanceCommandBuffer cb;
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)sb->mapped;
					auto offset = 0;
					for (auto i = 0; i < levels; i++)
					{
						auto size = gli_texture.size(i);
						auto ext = gli_texture.extent(i);
						for (auto j = 0; j < layers; j++)
						{
							void* data;
							if (faces > 1)
								data = gli_texture.data(0, j, i);
							else
								data = gli_texture.data(j, 0, i);
							memcpy(dst + offset, data, size);

							BufferImageCopy cpy;
							cpy.buf_off = offset;
							cpy.img_ext = ext;
							cpy.img_sub.base_level = i;
							cpy.img_sub.base_layer = j;
							cpies.push_back(cpy);

							offset += size;
						}
					}
					cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutTransferDst);
					cb->copy_buffer_to_image(sb.get(), ret, cpies);
					cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutShaderReadOnly);
					cb.excute();
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

					ret = Image::create(get_image_format(bmp->chs, bmp->bpp), uvec3(bmp->extent, 1),
						ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc | additional_usage, auto_mipmapping ? 0 : 1);

					{
						StagingBuffer sb(bmp->data_size, bmp->data);
						InstanceCommandBuffer cb;
						BufferImageCopy cpy;
						cpy.img_ext = ret->extent;
						cb->image_barrier(ret, {}, ImageLayoutTransferDst);
						cb->copy_buffer_to_image(sb.get(), ret, { &cpy, 1 });
						for (auto i = 1U; i < ret->n_levels; i++)
						{
							if (auto_mipmapping)
							{
								cb->image_barrier(ret, { i - 1, 1, 0, 1 }, ImageLayoutTransferSrc);
								cb->image_barrier(ret, { i, 1, 0, 1 }, ImageLayoutTransferDst);
								ImageBlit blit;
								blit.src_sub.base_level = i - 1;
								blit.src_range = ivec4(0, 0, ret->levels[i - 1].extent);
								blit.dst_sub.base_level = i;
								blit.dst_range = ivec4(0, 0, ret->levels[i].extent);
								cb->blit_image(ret, ret, { &blit, 1 }, FilterLinear);
								cb->image_barrier(ret, { i - 1, 1, 0, 1 }, ImageLayoutShaderReadOnly);
							}
						}
						cb->image_barrier(ret, { ret->n_levels - 1, 1, 0, 1 }, ImageLayoutShaderReadOnly);
						cb.excute();
					}

					if (auto_mipmapping && alpha_coverage > 0.f)
					{
						auto alpha_test = alpha_coverage;
						auto coverage = get_image_alphatest_coverage(ret, 0, alpha_test, 1.f);
						for (auto i = 1; i < ret->n_levels; i++)
							scale_image_alphatest_coverage(ret, i, coverage, alpha_test);
					}
				}

				ret->filename = filename;
				ret->srgb = srgb;
				ret->ref = 1;
				LoadedImage i;
				i.v.reset(ret);
				i.srgb = srgb;
				loaded_images.push_back(std::move(i));
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
						return i.v.get() == image;
					});
				}
				else
					image->ref--;
			}
		}Image_release;
		Image::Release& Image::release = Image_release;

		ImagePtr ImagePrivate::create(DevicePtr device, Format format, const uvec3& extent, VkImage native)
		{
			auto ret = new ImagePrivate;

			ret->format = format;
			ret->extent = extent;
			ret->initialize();
			ret->vk_image = native;
			register_object(ret->vk_image, "Image", ret);

			return ret;
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			if (app_exiting) return;

			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
			unregister_object(vk_image_view);
		}

		SamplerPrivate::~SamplerPrivate()
		{
			if (app_exiting) return;

			vkDestroySampler(device->vk_device, vk_sampler, nullptr);
			unregister_object(vk_sampler);
		}

		struct SamplerGet : Sampler::Get
		{
			SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, BorderColor border_color) override
			{
				for (auto& s : samplers)
				{
					if (s->mag_filter == mag_filter && s->min_filter == min_filter && s->linear_mipmap == linear_mipmap && s->address_mode == address_mode && s->border_color == border_color)
						return s.get();
				}

				auto ret = new SamplerPrivate;
				ret->mag_filter = mag_filter;
				ret->min_filter = min_filter;
				ret->linear_mipmap = linear_mipmap;
				ret->address_mode = address_mode;
				ret->border_color = border_color;

				VkSamplerCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				info.magFilter = to_backend(mag_filter);
				info.minFilter = to_backend(min_filter);
				info.mipmapMode = linear_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
				info.addressModeU = info.addressModeV = info.addressModeW = to_backend(address_mode);
				info.maxAnisotropy = 1.f;
				info.maxLod = VK_LOD_CLAMP_NONE;
				info.compareOp = VK_COMPARE_OP_ALWAYS;
				info.borderColor = to_backend(border_color);

				chk_res(vkCreateSampler(device->vk_device, &info, nullptr, &ret->vk_sampler));
				register_object(ret->vk_sampler, "Sampler", ret);

				samplers.emplace_back(ret);
				return ret;
			}
		}Sampler_get;
		Sampler::Get& Sampler::get = Sampler_get;
	}
}

