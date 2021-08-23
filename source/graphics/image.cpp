#include "../foundation/bitmap.h"
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "command_private.h"
#include "shader_private.h"

#include <gli/gli.hpp>

namespace flame
{
	namespace graphics
	{
		vec4 ImagePrivate::Data::get_pixel(ivec2 pos)
		{
			pos.x = clamp(pos.x, 0, (int)size.x - 1);
			pos.y = clamp(pos.y, 0, (int)size.y - 1);

			auto pixel = p.get() + pitch * pos.y + pixel_size * pos.x;
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
				fassert(0);
			}
		}

		void ImagePrivate::Data::set_pixel(ivec2 pos, const vec4& v)
		{
			pos.x = clamp(pos.x, 0, (int)size.x - 1);
			pos.y = clamp(pos.y, 0, (int)size.y - 1);

			auto pixel = p.get() + pitch * pos.y + pixel_size * pos.x;
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
				fassert(0);
			}
		}

		std::vector<ImagePrivate*> __images; // for debug: get image ptr from vulkan handle

		void ImagePrivate::initialize(const uvec2& size)
		{
			auto auto_lvs = false;
			if (levels == 0)
			{
				auto_lvs = true;
				levels = 16;
			}
			auto s = size;
			for (auto i = 0; i < levels; i++)
			{
				sizes.push_back(s);
				s.x /= 2;
				s.y /= 2;
				if (auto_lvs && s.x == 0 && s.y == 0)
				{
					levels = i + 1;
					break;
				}
				if (s.x == 0)
					s.x = 1;
				if (s.y == 0)
					s.y = 1;
			}

			auto pixel_size = get_pixel_size(format);
			data_size = 0;
			data.resize(levels);
			for (auto lv = 0; lv < levels; lv++)
			{
				data[lv].resize(layers);
				for (auto& d : data[lv])
				{
					d.format = format;
					d.size = sizes[lv];
					d.pixel_size = pixel_size;
					d.pitch = image_pitch(pixel_size * d.size.x);
					d.data_size =  d.pitch * d.size.y;
					data_size += d.data_size;
				}
			}
		}

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint lvs, uint layers, SampleCount sample_count, 
			ImageUsageFlags usage, bool is_cube) :
			device(device),
			format(format),
			levels(lvs),
			layers(layers),
			sample_count(sample_count),
			usage(usage),
			is_cube(is_cube)
		{
			__images.push_back(this);

			initialize(size);

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

			chk_res(vkCreateImage(device->vk_device, &imageInfo, nullptr, &vk_image));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(device->vk_device, vk_image, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = device->find_memory_type(memRequirements.memoryTypeBits, MemoryPropertyDevice);

			chk_res(vkAllocateMemory(device->vk_device, &allocInfo, nullptr, &vk_memory));

			chk_res(vkBindImageMemory(device->vk_device, vk_image, vk_memory, 0));
		}

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, void* native) :
			device(device),
			format(format),
			levels(levels),
			layers(layers),
			sample_count(SampleCount_1),
			usage(usage)
		{
			__images.push_back(this);

			initialize(size);

			vk_image = (VkImage)native;
		}

		ImagePrivate::~ImagePrivate()
		{
			std::erase_if(__images, [&](const auto& i) {
				return i == this;
			});

			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
			}
		}

		void ImagePrivate::release()
		{
			if (!filename.empty())
			{
				auto& texs = device->texs[srgb ? 1 : 0];
				for (auto it = texs.begin(); it != texs.end(); it++)
				{
					auto& tex = *it;
					if (tex.second.get() == this)
					{
						tex.first--;
						if (tex.first == 0)
							texs.erase(it);
						return;
					}
				}
			}
			else
				delete this;
		}

		ImagePrivate::Data& ImagePrivate::get_data(uint level, uint layer)
		{
			auto& d = data[level][layer];
			if (!d.p)
			{
				StagingBuffer sb(device, d.data_size, nullptr, BufferUsageTransferDst);
				{
					InstanceCB cb(device);
					BufferImageCopy cpy;
					cpy.img_ext = d.size;
					cpy.img_sub = { level, 1, layer, 1 };
					cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
					cb->copy_image_to_buffer(this, (BufferPrivate*)sb.get(), 1, &cpy);
					cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
				}
				d.p.reset(new uchar[d.data_size]);
				memcpy(d.p.get(), sb.mapped, d.data_size);
			}
			return d;
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

			auto iv = new ImageViewPrivate(this, sub, swizzle);
			views.emplace(key, iv);
			return iv;
		}

		static DescriptorSetLayoutPrivate* simple_dsl = nullptr;

		DescriptorSetPtr ImagePrivate::get_shader_read_src(uint base_level, uint base_layer, SamplerPtr sp)
		{
			if (!sp)
				sp = SamplerPrivate::get(device, FilterLinear, FilterLinear, false, AddressClampToEdge);

			auto key = (base_level & 0xff) << 24;
			key |= (base_layer & 0xff) << 16;
			key |= (uint)sp & 0xffff;

			auto it = read_dss.find(key);
			if (it != read_dss.end())
				return it->second.get();

			if (!simple_dsl)
			{
				DescriptorBindingInfo b;
				b.type = DescriptorSampledImage;
				simple_dsl = new DescriptorSetLayoutPrivate(device, { &b, 1 });
			}

			auto ds = new DescriptorSetPrivate(device->dsp.get(), simple_dsl);
			ds->set_image(0, 0, get_view({ base_level, 1, base_layer, 1 }), sp);
			ds->update();
			read_dss.emplace(key, ds);
			return ds;
		}

		static std::vector<RenderpassPrivate*> simple_rps;

		FramebufferPtr ImagePrivate::get_shader_write_dst(uint base_level, uint base_layer, bool clear)
		{
			auto key = (base_level & 0xff) << 24;
			key |= (base_layer & 0xff) << 16;
			key |= (uint)clear & 0xffff;

			auto it = write_fbs.find(key);
			if (it != write_fbs.end())
				return it->second.get();

			RenderpassPrivate* rp = nullptr;
			auto load_op = clear ? AttachmentLoadClear : AttachmentLoadDontCare;
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
				RenderpassAttachmentInfo att;
				att.format = format;
				att.load_op = load_op;
				att.sample_count = sample_count;
				RenderpassSubpassInfo sp;
				int col_ref = 0;
				if (format >= Format_Color_Begin && format <= Format_Color_End)
				{
					sp.color_attachments_count = 1;
					sp.color_attachments = &col_ref;
				}
				else
					sp.depth_attachment = 0;
				rp = new RenderpassPrivate(device, { &att, 1 }, { &sp, 1 });
				simple_rps.push_back(rp);
			}

			ImageViewPrivate* vs[] = { get_view({ base_level, 1, base_layer, 1 }) };
			auto fb = new FramebufferPrivate(device, rp, vs);
			write_fbs.emplace(key, fb);
			return fb;
		}

		void ImagePrivate::change_layout(ImageLayout src_layout, ImageLayout dst_layout)
		{
			InstanceCB cb(device);

			cb->image_barrier(this, { 0, levels, 0, layers }, src_layout, dst_layout);
		}

		void ImagePrivate::clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color)
		{
			InstanceCB cb(device);

			cb->image_barrier(this, { 0, levels, 0, layers }, src_layout, ImageLayoutTransferDst);
			cb->clear_color_image(this, { 0, levels, 0, layers }, color);
			cb->image_barrier(this, { 0, levels, 0, layers }, ImageLayoutTransferDst, dst_layout);
		}

		vec4 ImagePrivate::linear_sample(const vec2& uv, uint level, uint layer)
		{
			auto sz = sizes[level];
			auto& d = get_data(level, layer);

			auto coord = uv * vec2(sz) - 0.5f;
			auto coordi = ivec2(floor(coord));
			auto coordf = coord - vec2(coordi);
			
			return mix(
				mix(d.get_pixel(coordi + ivec2(0, 0)), d.get_pixel(coordi + ivec2(1, 0)), coordf.x),
				mix(d.get_pixel(coordi + ivec2(0, 1)), d.get_pixel(coordi + ivec2(1, 1)), coordf.x),
				coordf.y);
		}

		void ImagePrivate::generate_mipmaps()
		{
			fassert(levels == 1);

			auto s = sizes[0];
			for (auto i = 0; ; i++)
			{
				s.x >>= 1;
				s.y >>= 1;
				if (s.x == 0 && s.y == 0)
					break;
				levels++;
			}

			auto img = new ImagePrivate(device, format, sizes[0], levels, layers, sample_count, usage, is_cube);

			{
				InstanceCB cb(device);

				cb->image_barrier(this, {}, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
				cb->image_barrier(img, { 0, levels, 0, layers }, ImageLayoutUndefined, ImageLayoutTransferDst);
				{
					ImageCopy cpy;
					cpy.size = sizes[0];
					cb->copy_image(this, img, 1, &cpy);
				}
				for (auto i = 1U; i < levels; i++)
				{
					cb->image_barrier(img, { i - 1, 1, 0, 1 }, ImageLayoutTransferDst, ImageLayoutTransferSrc);
					ImageBlit blit;
					blit.src_sub.base_level = i - 1;
					blit.src_range = ivec4(0, 0, img->sizes[i - 1]);
					blit.dst_sub.base_level = i;
					blit.dst_range = ivec4(0, 0, img->sizes[i]);
					cb->blit_image(img, img, 1, &blit, FilterLinear);
					cb->image_barrier(img, { i - 1, 1, 0, 1 }, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
				}
				cb->image_barrier(img, { img->levels - 1, 1, 0, 1 }, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}

			data = std::move(img->data);
			data_size = img->data_size;
			std::swap(vk_image, img->vk_image);
			std::swap(vk_memory, img->vk_memory);
			delete img;
		}

		float ImagePrivate::alpha_test_coverage(uint level, float ref, uint channel, float scale)
		{
			fassert(format == Format_R8G8B8A8_UNORM || format == Format_R8_UNORM);

			auto& d = get_data(level, 0);

			auto coverage = 0.f;

			for (auto y = 0; y < d.size.y; y++)
			{
				for (auto x = 0; x < d.size.x; x++)
				{
					if (d.get_pixel(ivec2(x, y))[channel] * scale > ref)
						coverage += 1.f;
				}
			}

			return coverage / float(d.size.x * d.size.y);
		}

		void ImagePrivate::scale_alpha_to_coverage(uint level, float desired, float ref, uint channel)
		{
			auto min_alpha_scale = 0.f;
			auto max_alpha_scale = 4.f;
			auto alpha_scale = 1.f;
			auto best_alpha_scale = 1.f;
			auto best_error = std::numeric_limits<float>::max();

			for (int i = 0; i < 10; i++)
			{
				auto current_coverage = alpha_test_coverage(level, ref, channel, alpha_scale);

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

			auto& d = get_data(level, 0);
			for (auto y = 0; y < d.size.y; y++)
			{
				for (auto x = 0; x < d.size.x; x++)
				{
					auto pos = ivec2(x, y);
					auto v = d.get_pixel(pos);
					v[channel] *= best_alpha_scale;
					d.set_pixel(pos, v);
				}
			}

			{
				StagingBuffer sb(device, d.data_size, d.p.get());
				InstanceCB cb(device);
				BufferImageCopy cpy;
				cpy.img_ext = d.size;
				cpy.img_sub.base_level = level;
				cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
				cb->copy_buffer_to_image((BufferPrivate*)sb.get(), this, 1, &cpy);
				cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}
		}

		void ImagePrivate::save(const std::filesystem::path& filename)
		{
			fassert(usage & ImageUsageTransferSrc);

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
				fassert(gli_fmt != gli::FORMAT_UNDEFINED);
				
				auto gli_texture = gli::texture(gli::TARGET_2D, gli_fmt, ivec3(sizes[0], 1), layers, 1, levels);

				StagingBuffer sb(device, data_size, nullptr);
				std::vector<std::tuple<void*, void*, uint>> gli_cpies;
				{
					InstanceCB cb(device);
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)sb.mapped;
					auto offset = 0;
					for (auto i = 0; i < layers; i++)
					{
						for (auto j = 0; j < levels; j++)
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
					cb->image_barrier(this, { 0, levels, 0, layers }, ImageLayoutUndefined, ImageLayoutTransferSrc);
					cb->copy_image_to_buffer(this, (BufferPrivate*)sb.get(), cpies.size(), cpies.data());
					cb->image_barrier(this, { 0, levels, 0, layers }, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
				}
				for (auto& c : gli_cpies)
					memcpy(std::get<0>(c), std::get<1>(c), std::get<2>(c));

				gli::save(gli_texture, filename.string());
			}
			else
			{

			}
		}

		ImagePrivate* ImagePrivate::create(DevicePrivate* device, Bitmap* bmp)
		{
			auto i = new ImagePrivate(device, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()),
				uvec2(bmp->get_width(), bmp->get_height()), 1, 1, 
				SampleCount_1, ImageUsageSampled | ImageUsageStorage | ImageUsageTransferDst | ImageUsageTransferSrc);

			StagingBuffer sb(device, bmp->get_size(), bmp->get_data());
			InstanceCB cb(device);
			BufferImageCopy cpy;
			cpy.img_ext = i->sizes[0];
			cb->image_barrier(i, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image((BufferPrivate*)sb.get(), i, 1, &cpy);
			cb->image_barrier(i, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			return i;
		}

		ImagePrivate* ImagePrivate::get(DevicePrivate* device, const std::filesystem::path& filename, bool srgb)
		{
			auto& texs = device->texs[srgb ? 1 : 0];

			for (auto& tex : texs)
			{
				if (tex.second->filename == filename)
				{
					tex.first++;
					return tex.second.get();
				}
			}

			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find image: %s\n", filename.c_str());
				return nullptr;
			}

			ImagePtr ret = nullptr;

			auto ext = filename.extension();
			if (ext == L".ktx" || ext == L".dds")
			{
				auto is_cube = false;

				auto gli_texture = gli::load(filename.string());

				auto ext = gli_texture.extent();
				auto levels = (uint)gli_texture.levels();
				auto layers = (uint)gli_texture.layers();
				auto faces = (uint)gli_texture.faces();
				if (faces > 1)
				{
					fassert(layers == 1);
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
				fassert(format != Format_Undefined);

				ret = new ImagePrivate(device, format, ext, levels, layers,
					SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc, is_cube);

				StagingBuffer sb(device, ret->data_size, nullptr);
				InstanceCB cb(device);
				std::vector<BufferImageCopy> cpies;
				auto dst = (char*)sb.mapped;
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
				cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutUndefined, ImageLayoutTransferDst);
				cb->copy_buffer_to_image((BufferPrivate*)sb.get(), ret, cpies.size(), cpies.data());
				cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}
			else
			{
				UniPtr<Bitmap> bmp(Bitmap::create(filename.c_str()));
				if (srgb)
					bmp->srgb_to_linear();

				ret = create(device, bmp.get());
				ret->filename = filename;
				ret->srgb = srgb;
			}

			texs.emplace_back(1, ret);

			return ret;
		}

		Image* Image::create(Device* device, Format format, const uvec2& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool is_cube) 
		{ 
			return new ImagePrivate((DevicePrivate*)device, format, size, level, layer, sample_count, usage, is_cube);
		}

		Image* Image::create(Device* device, Bitmap* bmp) 
		{ 
			return ImagePrivate::create((DevicePrivate*)device, bmp);
		}

		Image* Image::get(Device* device, const wchar_t* filename, bool srgb) 
		{ 
			return ImagePrivate::get((DevicePrivate*)device, filename, srgb);
		}

		ImageViewPrivate::ImageViewPrivate(ImagePrivate* image, const ImageSub& sub, const ImageSwizzle& swizzle) :
			image(image),
			device(image->device),
			sub(sub),
			swizzle(swizzle)
		{
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = to_backend(swizzle.r);
			info.components.g = to_backend(swizzle.g);
			info.components.b = to_backend(swizzle.b);
			info.components.a = to_backend(swizzle.a);
			info.image = image->vk_image;
			if (image->is_cube && sub.base_layer == 0 && sub.layer_count == 6)
				info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			else if (sub.layer_count > 1)
				info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			else
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = to_backend(image->format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(image->format));
			info.subresourceRange.baseMipLevel = sub.base_level;
			info.subresourceRange.levelCount = sub.level_count;
			info.subresourceRange.baseArrayLayer = sub.base_layer;
			info.subresourceRange.layerCount = sub.layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &vk_image_view));
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
		}

		SamplerPrivate::SamplerPrivate(DevicePrivate* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode) :
			device(device),
			mag_filter(mag_filter),
			min_filter(min_filter),
			linear_mipmap(linear_mipmap),
			address_mode(address_mode)
		{
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

			chk_res(vkCreateSampler(device->vk_device, &info, nullptr, &vk_sampler));
		}

		SamplerPrivate::~SamplerPrivate()
		{
			vkDestroySampler(device->vk_device, vk_sampler, nullptr);
		}

		SamplerPrivate* SamplerPrivate::get(DevicePrivate* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode)
		{
			for (auto& s : device->sps)
			{
				if (s->mag_filter == mag_filter && s->min_filter == min_filter && s->linear_mipmap == linear_mipmap && s->address_mode == address_mode)
					return s.get();
			}
			auto s = new SamplerPrivate(device, mag_filter, min_filter, linear_mipmap, address_mode);
			device->sps.emplace_back(s);
			return s;
		}

		Sampler* Sampler::get(Device* device, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode)
		{
			return SamplerPrivate::get((DevicePrivate*)device, mag_filter, min_filter, linear_mipmap, address_mode);
		}

		ImageAtlasPrivate::ImageAtlasPrivate(DevicePrivate* device, const std::filesystem::path& fn)
		{
			auto filename = fn;
			auto ini = parse_ini_file(filename);

			filename.replace_extension(L".png");
			image = ImagePrivate::get(device, filename.c_str(), false);

			auto w = (float)image->sizes[0].x;
			auto h = (float)image->sizes[0].y;

			for (auto& e : ini.get_section_entries(""))
			{
				Tile tile;
				tile.index = tiles.size();
				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile.name = t;
				ss >> t;
				auto v = sto<4, uint>(t.c_str());
				tile.pos = ivec2(v.x, v.y);
				tile.size = ivec2(v.z, v.w);
				tile.uv.x = tile.pos.x / w;
				tile.uv.y = tile.pos.y / h;
				tile.uv.z = (tile.pos.x + tile.size.x) / w;
				tile.uv.w = (tile.pos.y + tile.size.y) / h;

				tiles.push_back(tile);
			}
		}

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			delete image;
		}

		void ImageAtlasPrivate::get_tile(uint id, TileInfo* dst) const
		{
			if (id >= tiles.size())
				return;
			auto& src = tiles[id];
			dst->id = id;
			dst->name = src.name.c_str();
			dst->pos = src.pos;
			dst->size = src.size;
			dst->uv = src.uv;
		}

		int ImageAtlasPrivate::find_tile(const std::string& name) const
		{
			for (auto id = 0; id < tiles.size(); id++)
			{
				if (tiles[id].name == name)
					return id;
			}
			return -1;
		}

		bool ImageAtlasPrivate::find_tile(const char* name, TileInfo* dst) const
		{
			auto id = find_tile(std::string(name));
			if (id != -1)
			{
				get_tile(id, dst);
				return true;
			}
			return false;
		}

		static std::vector<std::pair<std::filesystem::path, UniPtr<ImageAtlasPrivate>>> loaded_atlas;

		ImageAtlas* ImageAtlas::get(Device* device, const wchar_t* fn)
		{
			auto filename = std::filesystem::path(fn);
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

			return new ImageAtlasPrivate((DevicePrivate*)device, filename);
		}
	}
}

