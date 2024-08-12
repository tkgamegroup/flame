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
		std::vector<std::pair<std::unique_ptr<ImageT>, ImageConfig>> loaded_images;
		std::vector<std::unique_ptr<SamplerT>> shared_samplers;
		std::vector<std::unique_ptr<ImageAtlasT>> loaded_image_atlases;

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

#if USE_D3D12
			d3d12_resource->Release();
			unregister_object(d3d12_resource);
#elif USE_VULKAN
			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
			}
			unregister_object(vk_image);
			unregister_object(vk_memory);
#endif
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

		ImageViewPtr ImagePrivate::get_view(const ImageSub& sub, const ImageSwizzle& swizzle, bool is_cube)
		{
			uint64 key;
			{
				uint hi; uint lo;
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
			iv->is_cube = is_cube;

#if USE_VULKAN
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = to_vk(swizzle.r);
			info.components.g = to_vk(swizzle.g);
			info.components.b = to_vk(swizzle.b);
			info.components.a = to_vk(swizzle.a);
			info.image = vk_image;
			if (is_cube)
			{
				assert(sub.base_layer == 0 && sub.layer_count == 6);
				info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
			}
			else if (sub.layer_count > 1)
				info.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			else if (extent.z > 1)
				info.viewType = VK_IMAGE_VIEW_TYPE_3D;
			else
				info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			info.format = to_vk(format);
			info.subresourceRange.aspectMask = to_vk_flags<ImageAspectFlags>(aspect_from_format(format));
			info.subresourceRange.baseMipLevel = sub.base_level;
			info.subresourceRange.levelCount = sub.level_count;
			info.subresourceRange.baseArrayLayer = sub.base_layer;
			info.subresourceRange.layerCount = sub.layer_count;

			check_vk_result(vkCreateImageView(device->vk_device, &info, nullptr, &iv->vk_image_view));
			register_object(iv->vk_image_view, "Image View", iv);
#endif

			views.emplace(key, iv);
			return iv;
		}

		static DescriptorSetLayoutPrivate* dummy_dsl = nullptr;

		DescriptorSetPtr ImagePrivate::get_shader_read_src(uint base_level, uint base_layer, SamplerPtr sp, const ImageSwizzle& swizzle)
		{
			if (!sp)
				sp = SamplerPrivate::get(FilterLinear, FilterLinear, false, AddressClampToEdge);

			uint64 key;
			{
				uint hi;
				hi = (base_level & 0xff) << 24;
				hi |= (base_layer & 0xff) << 16;
				hi |= swizzle.r << 12;
				hi |= swizzle.g << 8;
				hi |= swizzle.b << 4;
				hi |= swizzle.a;

				uint lo = (uint)sp;
				key = (((uint64)hi) << 32) | ((uint64)lo);
			}

			auto it = read_dss.find(key);
			if (it != read_dss.end())
				return it->second.get();

			if (!dummy_dsl)
			{
				DescriptorBinding b;
				b.type = DescriptorSampledImage;
				dummy_dsl = DescriptorSetLayout::create({ &b, 1 });
			}

			auto ds = DescriptorSet::create(nullptr, dummy_dsl);
			ds->set_image_i(0, 0, get_view({ base_level, 1, base_layer, 1 }, swizzle), sp);
			ds->update();
			read_dss.emplace(key, ds);
			return ds;
		}

		FramebufferPtr ImagePrivate::get_shader_write_dst(uint base_level, uint base_layer, AttachmentLoadOp load_op)
		{
			static std::vector<RenderpassPrivate*> dummy_rps;

			auto key = (base_level & 0xff) << 24;
			key |= (base_layer & 0xff) << 16;
			key |= (uint)load_op & 0xffff;

			auto it = write_fbs.find(key);
			if (it != write_fbs.end())
				return it->second.get();

			RenderpassPrivate* rp = nullptr;
			for (auto& r : dummy_rps)
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
					sp.depth_stencil_attachment = 0;
				rp = Renderpass::create(info);
				dummy_rps.push_back(rp);
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
			if (format >= Format_Depth_Begin && format <= Format_Depth_End)
				cb->clear_depth_image(this, { 0, n_levels, 0, n_layers }, color.x);
			else
				cb->clear_color_image(this, { 0, n_levels, 0, n_layers }, color);
			cb->image_barrier(this, { 0, n_levels, 0, n_layers }, dst_layout);
			cb.excute();
		}

		vec4 pixel_to_vec4(Format format, uchar* pixel)
		{
			switch (format)
			{
			case Format_R8_UNORM:
				return vec4(pixel[0] / 255.f, 0.f, 0.f, 0.f);
			case Format_R8G8B8A8_UNORM:
				return vec4(pixel[0] / 255.f, pixel[1] / 255.f, pixel[2] / 255.f, pixel[3] / 255.f);
			case Format_R16G16B16A16_SFLOAT:
				return vec4(unpackHalf1x16(((ushort*)pixel)[0]), unpackHalf1x16(((ushort*)pixel)[1]),
					unpackHalf1x16(((ushort*)pixel)[2]), unpackHalf1x16(((ushort*)pixel)[3]));
			case Format_Depth16:
				return vec4(unpackUnorm1x16(((ushort*)pixel)[0]), 0.f, 0.f, 0.f);
			case Format_R32_SFLOAT:
			case Format_Depth32:
				return vec4(((float*)pixel)[0], 0.f, 0.f, 0.f);
			}
			return vec4(0.f);
		}

		vec4 ImagePrivate::get_pixel(int x, int y, uint level, uint layer)
		{
			if (!(usage & ImageUsageTransferSrc))
				return vec4(0.f);

			auto& lv = levels[level];
			StagingBuffer sb(pixel_size);
			InstanceCommandBuffer cb;
			BufferImageCopy cpy;
			cpy.img_off = uvec3(x, y, 0);
			cpy.img_ext = uvec3(1);
			cpy.img_sub = { level, 1, layer, 1 };
			cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(this, sb.get(), { &cpy, 1 });
			cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly);
			cb.excute();
			return pixel_to_vec4(format, (uchar*)sb->mapped);
		}

		void ImagePrivate::stage_surface_data(uint level, uint layer)
		{
			if (!(usage & ImageUsageTransferSrc))
				return;

			auto& lv = levels[level];
			auto& ly = lv.layers[layer];
			if (!ly.staging_data)
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
				ly.staging_data.reset(new uchar[lv.data_size]);
				memcpy(ly.staging_data.get(), sb->mapped, lv.data_size);
			}
		}

		vec4 ImagePrivate::get_staging_pixel(int x, int y, uint level, uint layer)
		{
			if (!(usage & ImageUsageTransferSrc))
				return vec4(0.f);

			stage_surface_data(level, layer);

			auto& lv = levels[level];
			auto& ly = lv.layers[layer];

			x = clamp(x, 0, (int)lv.extent.x - 1);
			y = clamp(y, 0, (int)lv.extent.y - 1);

			auto pixel = ly.staging_data.get() + lv.pitch * y + pixel_size * x;
			return pixel_to_vec4(format, pixel);
		}

		void ImagePrivate::set_staging_pixel(int x, int y, uint level, uint layer, const vec4& v)
		{
			if (!(usage & ImageUsageTransferDst))
				return;

			stage_surface_data(level, layer);

			auto& lv = levels[level];
			auto& ly = lv.layers[layer];

			x = clamp(x, 0, (int)lv.extent.x - 1);
			y = clamp(y, 0, (int)lv.extent.y - 1);

			auto pixel = ly.staging_data.get() + lv.pitch * y + pixel_size * x;
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

		void ImagePrivate::upload_staging_pixels(int x, int y, int w, int h, uint level, uint layer)
		{
			if (!(usage & ImageUsageTransferDst))
				return;

			auto& lv = levels[level];
			auto& ly = lv.layers[layer];
			if (ly.staging_data)
			{
				auto img_line_size = image_pitch(lv.extent.x * pixel_size);
				auto copy_line_size = image_pitch(w * pixel_size);
				StagingBuffer sb(copy_line_size * h);
				auto src = ly.staging_data.get() + y * img_line_size + x * pixel_size;
				auto dst = (uchar*)sb->mapped;
				for (auto i = 0; i < h; i++)
				{
					memcpy(dst, src, copy_line_size);
					src += img_line_size;
					dst += copy_line_size;
				}

				InstanceCommandBuffer cb;
				BufferImageCopy cpy;
				cpy.img_off = uvec3(x, y, 0);
				cpy.img_ext = uvec3(w, h, 1);
				cpy.img_sub = { level, 1, layer, 1 };
				cb->image_barrier(this, cpy.img_sub, ImageLayoutTransferDst);
				cb->copy_buffer_to_image(sb.get(), this, { &cpy, 1 });
				cb->image_barrier(this, cpy.img_sub, ImageLayoutShaderReadOnly);
				cb.excute();
			}
		}

		void ImagePrivate::clear_staging_pixels()
		{
			for (auto level = 0; level < n_levels; level++)
			{
				auto& lv = levels[level];
				for (auto layer = 0; layer < n_layers; layer++)
				{
					auto& ly = lv.layers[layer];
					ly.staging_data.reset();
				}
			}
		}

		vec4 ImagePrivate::linear_sample_staging_pixels(const vec2& uv, uint level, uint layer)
		{
			auto coord = uv * vec2(levels[level].extent) - 0.5f;
			auto coordi = ivec2(floor(coord));
			auto coordf = coord - vec2(coordi);

			return mix(
				mix(get_staging_pixel(coordi.x, coordi.y, level, layer), get_staging_pixel(coordi.x + 1, coordi.y, level, layer), coordf.x),
				mix(get_staging_pixel(coordi.x, coordi.y + 1, level, layer), get_staging_pixel(coordi.x + 1, coordi.y + 1, level, layer), coordf.x),
				coordf.y);
		}

		void ImagePrivate::save(const std::filesystem::path& filename, bool compress)
		{
			assert(usage & ImageUsageTransferSrc);

			auto ext = filename.extension();
			if (ext == L".dds")
			{
				auto gli_target = gli::TARGET_2D;
				if (n_layers == 6)
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
				case Format_Depth16:
					gli_fmt = gli::FORMAT_R16_UNORM_PACK16;
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
						memcpy(gli_texture.data(j, 0, i), levels[i].layers[j].staging_data.get(), size);
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
					if (img->get_staging_pixel(x, y, level, 0)[ch] * scale > ref)
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
					auto v = img->get_staging_pixel(x, y, level, 0);
					v[ch] *= best_alpha_scale;
					img->set_staging_pixel(x, y, level, 0, v);
				}
			}

			StagingBuffer sb(lv.data_size, ly.staging_data.get());
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
			ImagePtr operator()(Format format, const uvec3& extent, ImageUsageFlags usage, uint levels, uint layers, SampleCount sample_count) override
			{
				uint u;
				auto additional_usage_transfer = device->get_config("image_additional_usage_transfer"_h, u) ? u == 1 : false;
				if (additional_usage_transfer)
					usage = usage | ImageUsageTransferSrc | ImageUsageTransferDst;

				auto is_cube = (layers == (uint)-6);
				if (is_cube)
					layers = 6;

				auto ret = new ImagePrivate;
				ret->format = format;
				ret->n_levels = levels;
				ret->n_layers = layers;
				ret->sample_count = sample_count;
				ret->usage = usage;
				ret->extent = extent;
				ret->initialize();

#if USE_D3D12
				D3D12_HEAP_PROPERTIES heap_properties = {};
				heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
				heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heap_properties.CreationNodeMask = 1;
				heap_properties.VisibleNodeMask = 1;
				D3D12_RESOURCE_DESC desc = {};
				desc.Dimension = extent.z > 1 ? D3D12_RESOURCE_DIMENSION_TEXTURE3D : D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				desc.Alignment = 0;
				desc.Width = extent.x;
				desc.Height = extent.y;
				desc.DepthOrArraySize = desc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D ? extent.z : ret->n_layers;
				desc.MipLevels = ret->n_levels;
				desc.Format = to_dx(format);
				switch (sample_count)
				{
				case SampleCount_2:
					desc.SampleDesc.Count = 2;
					desc.SampleDesc.Quality = 1;
					break;
				case SampleCount_4:
					desc.SampleDesc.Count = 4;
					desc.SampleDesc.Quality = 1;
					break;
				case SampleCount_8:
					desc.SampleDesc.Count = 8;
					desc.SampleDesc.Quality = 1;
					break;
				case SampleCount_16:
					desc.SampleDesc.Count = 16;
					desc.SampleDesc.Quality = 1;
					break;
				default:
					desc.SampleDesc.Count = 1;
					desc.SampleDesc.Quality = 0;
				}
				desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				desc.Flags = D3D12_RESOURCE_FLAG_NONE;
				D3D12_CLEAR_VALUE cv; D3D12_CLEAR_VALUE* pcv = nullptr;
				if (usage & ImageUsageAttachment)
				{
					if (format >= Format_Depth_Begin && format <= Format_Depth_End)
					{
						desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
						cv.Format = desc.Format;
						cv.DepthStencil.Depth = 1.f;
						cv.DepthStencil.Stencil = 0;
						pcv = &cv;
					}
					else
					{
						desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
						cv.Format = desc.Format;
						cv.Color[0] = 0;
						cv.Color[1] = 0;
						cv.Color[2] = 0;
						cv.Color[3] = 0;
						pcv = &cv;
					}
				}
				check_dx_result(device->d3d12_device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, pcv, IID_PPV_ARGS(&ret->d3d12_resource)));
				register_object(ret->d3d12_resource, "Image", this);

				auto img_desc = ret->d3d12_resource->GetDesc();
				for (auto i = 0; i < ret->n_layers; i++)
				{
					for (auto j = 0; j < ret->n_levels; j++)
					{
						auto sub_idx = (i * ret->n_levels) + j;
						D3D12_PLACED_SUBRESOURCE_FOOTPRINT sub_footprint;
						uint num_rows; uint64 row_size; uint64 required_size;
						device->d3d12_device->GetCopyableFootprints(&img_desc, sub_idx, 1, 0, &sub_footprint, &num_rows, &row_size, &required_size);
						ret->levels[j].pitch = sub_footprint.Footprint.RowPitch;
						ret->levels[j].data_size = required_size;
					}
				}
				ret->data_size = 0;
				for (auto& lv : ret->levels)
				{
					for (auto& ly : lv.layers)
						ly.layout = ImageLayoutGeneral;
					ret->data_size += lv.data_size * ret->n_layers;
				}
#elif USE_VULKAN
				VkImageCreateInfo imageInfo;
				imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageInfo.flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
				imageInfo.pNext = nullptr;
				if (layers == (uint)-6)
				{
					imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
					layers = 6;
				}
				imageInfo.imageType = extent.z > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
				imageInfo.format = to_vk(format);
				imageInfo.extent.width = extent.x;
				imageInfo.extent.height = extent.y;
				imageInfo.extent.depth = extent.z;
				imageInfo.mipLevels = ret->n_levels;
				imageInfo.arrayLayers = ret->n_layers;
				imageInfo.samples = to_vk(sample_count);
				imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageInfo.usage = from_backend_flags(usage, format, sample_count);
				imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				imageInfo.queueFamilyIndexCount = 0;
				imageInfo.pQueueFamilyIndices = nullptr;
				imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				check_vk_result(vkCreateImage(device->vk_device, &imageInfo, nullptr, &ret->vk_image));

				VkMemoryRequirements memRequirements;
				vkGetImageMemoryRequirements(device->vk_device, ret->vk_image, &memRequirements);

				VkMemoryAllocateInfo allocInfo;
				allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				allocInfo.pNext = nullptr;
				allocInfo.allocationSize = memRequirements.size;
				allocInfo.memoryTypeIndex = device->find_memory_type(memRequirements.memoryTypeBits, MemoryPropertyDevice);

				check_vk_result(vkAllocateMemory(device->vk_device, &allocInfo, nullptr, &ret->vk_memory));
				check_vk_result(vkBindImageMemory(device->vk_device, ret->vk_image, ret->vk_memory, 0));
				register_object(ret->vk_image, "Image", ret);
				register_object(ret->vk_memory, "Image Memory", ret);
#endif

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

		bool read_image_config(const std::filesystem::path& image_path, ImageConfig* out)
		{
			auto config_path = image_path;
			config_path += ".ini";
			if (std::filesystem::exists(config_path))
			{
				auto ini = parse_ini_file(config_path);
				for (auto& e : ini.get_section_entries(""))
				{
					if (e.key == "srgb")
						TypeInfo::unserialize_t(e.values[0], out->srgb);
					if (e.key == "auto_mipmapping")
						TypeInfo::unserialize_t(e.values[0], out->auto_mipmapping);
					if (e.key == "alpha_test")
						out->alpha_test = s2t<float>(e.values[0]);
					if (e.key == "border")
						out->border = s2t<4, float>(e.values[0]);
				}
				return true;
			}
			return false;
		}

		struct ImageGet : Image::Get
		{
			ImagePtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);
				auto ext = filename.extension();

				for (auto& i : loaded_images)
				{
					if (i.first->filename == filename)
					{
						i.first->ref++;
						return i.first.get();
					}
				}

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find image: %s\n", _filename.c_str());
					return nullptr;
				}

				ImageConfig config;
				read_image_config(filename, &config);

				ImageUsageFlags additional_usage = ImageUsageNone;

				ImagePtr ret = nullptr;

				if (ext == L".ktx" || ext == L".dds")
				{
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

					ret = Image::create(format, ext, ImageUsageSampled | ImageUsageTransferDst | additional_usage,
						levels, layers == 6 ? (uint)-6 : layers, SampleCount_1);
#if USE_D3D12
					for (auto& lv : ret->levels)
					{
						for (auto& ly : lv.layers)
							ly.layout = ImageLayoutTransferDst;
					}
#endif

					StagingBuffer sb(ret->data_size, nullptr);
					InstanceCommandBuffer cb;
					std::vector<BufferImageCopy> copies;
					auto dst = (char*)sb->mapped;
					auto offset = 0;
					for (auto i = 0; i < levels; i++)
					{
						auto size = ret->levels[i].data_size;
						auto ext = ret->levels[i].extent;
						auto src_pitch = image_pitch(ret->pixel_size * ext.x);
						auto dst_pitch = ret->levels[i].pitch;
						for (auto j = 0; j < layers; j++)
						{
							void* data;
							if (faces > 1)
								data = gli_texture.data(0, j, i);
							else
								data = gli_texture.data(j, 0, i);
#if USE_D3D12
							for (auto l = 0; l < ext.y; l++)
								memcpy(dst + offset + l * dst_pitch, (char*)data + l * src_pitch, src_pitch);
#else
							memcpy(dst + offset, data, size);
#endif

							BufferImageCopy cpy;
							cpy.buf_off = offset;
							cpy.img_ext = uvec3(ext, 1);
							cpy.img_sub.base_level = i;
							cpy.img_sub.base_layer = j;
							copies.push_back(cpy);

							offset += size;
						}
					}
					cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutTransferDst);
					cb->copy_buffer_to_image(sb.get(), ret, copies);
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

					if (config.srgb)	bmp->srgb_to_linear();
					if (bmp->chs == 3)	bmp->change_format(4);

					ret = Image::create(get_image_format(bmp->chs, bmp->bpp), uvec3(bmp->extent, 1),
						ImageUsageSampled | ImageUsageTransferDst | additional_usage, config.auto_mipmapping ? 0 : 1);

#if USE_D3D12
					StagingBuffer sb(ret->data_size, nullptr);
					auto ext = ret->levels[0].extent;
					auto src_pitch = image_pitch(ret->pixel_size * ext.x);
					auto dst_pitch = ret->levels[0].pitch;
					for (auto l = 0; l < ext.y; l++)
						memcpy((char*)sb->mapped + l * dst_pitch, (char*)bmp->data + l * src_pitch, src_pitch);
#else
					StagingBuffer sb(ret->data_size, bmp->data);
#endif
					InstanceCommandBuffer cb;
					BufferImageCopy cpy;
					cpy.img_ext = ret->extent;
					cb->image_barrier(ret, {}, ImageLayoutTransferDst);
					cb->copy_buffer_to_image(sb.get(), ret, { &cpy, 1 });
					if (config.auto_mipmapping)
					{
						for (auto i = 1U; i < ret->n_levels; i++)
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
						cb->image_barrier(ret, { ret->n_levels - 1, 1, 0, 1 }, ImageLayoutShaderReadOnly);
					}
					else
						cb->image_barrier(ret, {}, ImageLayoutShaderReadOnly);
					cb.excute();

					if (config.auto_mipmapping && config.alpha_test > 0.f)
					{
						auto coverage = get_image_alphatest_coverage(ret, 0, config.alpha_test, 1.f);
						for (auto i = 1; i < ret->n_levels; i++)
							scale_image_alphatest_coverage(ret, i, coverage, config.alpha_test);
					}
				}

				ret->filename = filename;
				ret->ref = 1;
				loaded_images.emplace_back(ret, config);
				return ret;
			}
		}Image_get;
		Image::Get& Image::get = Image_get;

		struct ImageGetConfig : Image::GetConfig
		{
			void operator()(const std::filesystem::path& _filename, ImageConfig* out) override
			{
				auto filename = Path::get(_filename);

				for (auto& i : loaded_images)
				{
					if (i.first->filename == filename)
					{
						*out = i.second;
						return;
					}
				}
			}
		}Image_get_config;
		Image::GetConfig& Image::get_config = Image_get_config;

		struct ImageRelease : Image::Release
		{
			void operator()(ImagePtr image) override
			{
				if (image->ref == 1)
				{
					graphics::Queue::get()->wait_idle();
					std::erase_if(loaded_images, [&](const auto& i) {
						return i.first.get() == image;
					});
				}
				else
					image->ref--;
			}
		}Image_release;
		Image::Release& Image::release = Image_release;

		ImagePtr ImagePrivate::create(DevicePtr device, Format format, const uvec3& extent, void* native)
		{
			auto ret = new ImagePrivate;

			ret->format = format;
			ret->extent = extent;
			ret->initialize();
#if USE_D3D12
			ret->d3d12_resource = (ID3D12Resource*)native;
			register_object(ret->d3d12_resource, "Image", ret);
#elif USE_VULKAN
			ret->vk_image = (VkImage)native;
			register_object(ret->vk_image, "Image", ret);
#endif

			return ret;
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			if (app_exiting) return;

#if USE_VULKAN
			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
			unregister_object(vk_image_view);
#endif
		}

		DescriptorSetPtr ImageViewPrivate::get_shader_read_src(SamplerPtr sp)
		{
			if (!sp)
				sp = SamplerPrivate::get(FilterLinear, FilterLinear, false, AddressClampToEdge);

			uint64 key;
			{
				uint hi;
				hi = (sub.base_level & 0xff) << 24;
				hi |= (sub.base_layer & 0xff) << 16;
				hi |= swizzle.r << 12;
				hi |= swizzle.g << 8;
				hi |= swizzle.b << 4;
				hi |= swizzle.a;

				uint lo = (uint)sp;
				key = (((uint64)hi) << 32) | ((uint64)lo);
			}

			auto it = image->read_dss.find(key);
			if (it != image->read_dss.end())
				return it->second.get();

			if (!dummy_dsl)
			{
				DescriptorBinding b;
				b.type = DescriptorSampledImage;
				dummy_dsl = DescriptorSetLayout::create({ &b, 1 });
			}

			auto ds = DescriptorSet::create(nullptr, dummy_dsl);
			ds->set_image_i(0, 0, this, sp);
			ds->update();
			image->read_dss.emplace(key, ds);
			return ds;
		}

		SamplerPrivate::~SamplerPrivate()
		{
			if (app_exiting) return;

#if USE_VULKAN
			vkDestroySampler(device->vk_device, vk_sampler, nullptr);
			unregister_object(vk_sampler);
#endif
		}

		void SamplerPrivate::copy_to(D3D12_CPU_DESCRIPTOR_HANDLE dst)
		{
			auto n = 1U;
			auto handle = d3d12_heap->GetCPUDescriptorHandleForHeapStart();
			device->d3d12_device->CopyDescriptors(1, &dst, &n, 1, &handle, &n, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
		}

#if USE_D3D12
		void create_dx_sampler(ID3D12DescriptorHeap* heap, Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, BorderColor border_color, float custom_border_color = 1.f)
		{
			D3D12_SAMPLER_DESC desc_sp = {};
			desc_sp.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			if (linear_mipmap)
			{
				if (min_filter == FilterNearest && mag_filter == FilterNearest)
					desc_sp.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
				else if (min_filter == FilterLinear && mag_filter == FilterNearest)
					desc_sp.Filter = D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
				else if (min_filter == FilterNearest && mag_filter == FilterLinear)
					desc_sp.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
				else
					desc_sp.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			}
			else
			{
				if (min_filter == FilterNearest && mag_filter == FilterNearest)
					desc_sp.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
				else if (min_filter == FilterLinear && mag_filter == FilterNearest)
					desc_sp.Filter = D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
				else if (min_filter == FilterNearest && mag_filter == FilterLinear)
					desc_sp.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
				else
					desc_sp.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			}
			switch (address_mode)
			{
			case AddressClampToEdge:
				desc_sp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
				break;
			case AddressClampToBorder:
				desc_sp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
				break;
			case AddressRepeat:
				desc_sp.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				break;
			}
			desc_sp.AddressV = desc_sp.AddressU;
			desc_sp.AddressW = desc_sp.AddressU;
			desc_sp.MipLODBias = 0;
			desc_sp.MaxAnisotropy = 0;
			desc_sp.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			switch (border_color)
			{
			case BorderColorBlack:
				desc_sp.BorderColor[0] = 0; desc_sp.BorderColor[1] = 0; desc_sp.BorderColor[2] = 0; desc_sp.BorderColor[3] = 1;
				break;
			case BorderColorWhite:
				desc_sp.BorderColor[0] = 1; desc_sp.BorderColor[1] = 1; desc_sp.BorderColor[2] = 1; desc_sp.BorderColor[3] = 1;
				break;
			case BorderColorTransparentBlack:
				desc_sp.BorderColor[0] = 0; desc_sp.BorderColor[1] = 0; desc_sp.BorderColor[2] = 0; desc_sp.BorderColor[3] = 0;
				break;
			case BorderColorTransparentWhite:
				desc_sp.BorderColor[0] = 1; desc_sp.BorderColor[1] = 1; desc_sp.BorderColor[2] = 1; desc_sp.BorderColor[3] = 0;
				break;
			case BorderColorCustom:
				desc_sp.BorderColor[0] = custom_border_color; desc_sp.BorderColor[1] = custom_border_color; desc_sp.BorderColor[2] = custom_border_color; desc_sp.BorderColor[3] = custom_border_color;
				break;
			}
			desc_sp.MinLOD = 0.0f;
			desc_sp.MaxLOD = D3D12_FLOAT32_MAX;
			auto handle = heap->GetCPUDescriptorHandleForHeapStart();
			device->d3d12_device->CreateSampler(&desc_sp, handle);
		}
#elif USE_VULKAN
		VkSampler create_vk_sampler(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, BorderColor border_color, float custom_border_color = 1.f)
		{
			VkSampler ret;

			VkSamplerCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.magFilter = to_vk(mag_filter);
			info.minFilter = to_vk(min_filter);
			info.mipmapMode = linear_mipmap ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.addressModeU = info.addressModeV = info.addressModeW = to_vk(address_mode);
			info.maxAnisotropy = 1.f;
			info.maxLod = VK_LOD_CLAMP_NONE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.borderColor = to_vk(border_color);
			if (border_color == BorderColorCustom)
			{
				static VkSamplerCustomBorderColorCreateInfoEXT vk_custom_border;
				vk_custom_border.sType = VK_STRUCTURE_TYPE_SAMPLER_CUSTOM_BORDER_COLOR_CREATE_INFO_EXT;
				vk_custom_border.pNext = nullptr;
				for (auto i = 0; i < 4; i++)
					vk_custom_border.customBorderColor.float32[i] = custom_border_color;
				vk_custom_border.format = VK_FORMAT_R32_SFLOAT;
				info.pNext = &vk_custom_border;
			}

			check_vk_result(vkCreateSampler(device->vk_device, &info, nullptr, &ret));
			register_object(ret, "Sampler", ret);

			return ret;
		}
#endif

		struct SamplerCreate : Sampler::Create
		{
			SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, float custom_border_color) override
			{
				auto ret = new SamplerPrivate;
				ret->mag_filter = mag_filter;
				ret->min_filter = min_filter;
				ret->linear_mipmap = linear_mipmap;
				ret->address_mode = address_mode;
				ret->border_color = BorderColorCustom;

#if USE_D3D12
				D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
				heap_desc.NumDescriptors = 1;
				heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
				heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				device->d3d12_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&ret->d3d12_heap));
				create_dx_sampler(ret->d3d12_heap, mag_filter, min_filter, linear_mipmap, address_mode, BorderColorCustom, custom_border_color);
#elif USE_VULKAN
				ret->vk_sampler = create_vk_sampler(mag_filter, min_filter, linear_mipmap, address_mode, BorderColorCustom, custom_border_color);
#endif

				return ret;
			}
		}Sampler_create;
		Sampler::Create& Sampler::create = Sampler_create;

		struct SamplerGet : Sampler::Get
		{
			SamplerPtr operator()(Filter mag_filter, Filter min_filter, bool linear_mipmap, AddressMode address_mode, BorderColor border_color) override
			{
				for (auto& s : shared_samplers)
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

#if USE_D3D12
				D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
				heap_desc.NumDescriptors = 1;
				heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
				heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				device->d3d12_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&ret->d3d12_heap));
				create_dx_sampler(ret->d3d12_heap, mag_filter, min_filter, linear_mipmap, address_mode, BorderColorCustom);
#elif USE_VULKAN
				ret->vk_sampler = create_vk_sampler(mag_filter, min_filter, linear_mipmap, address_mode, border_color);
#endif

				shared_samplers.emplace_back(ret);
				return ret;
			}
		}Sampler_get;
		Sampler::Get& Sampler::get = Sampler_get;

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			Image::release(image);
		}

		struct ImageAtlasGet : ImageAtlas::Get
		{
			ImageAtlasPtr operator()(const std::filesystem::path& _filename) override
			{
				auto filename = Path::get(_filename);

				for (auto& a : loaded_image_atlases)
				{
					if (a->filename == filename)
					{
						a->ref++;
						return a.get();
					}
				}

				auto atlas_ini_path = filename;
				atlas_ini_path += L".ini";
				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find image: %s\n", _filename.c_str());
					return nullptr;
				}
				if (!std::filesystem::exists(atlas_ini_path))
				{
					wprintf(L"cannot find image atlas ini: %s.ini\n", _filename.c_str());
					return nullptr;
				}

				auto img = Image::get(filename);
				if (!img)
				{
					wprintf(L"cannot load image: %s\n", _filename.c_str());
					return nullptr;
				}

				auto ret = new ImageAtlasPrivate;
				ret->image = img;
				ret->view = img->get_view({ 0, img->n_levels, 0, 1 });
				auto atlas_ini = parse_ini_file(atlas_ini_path);
				for (auto& s : atlas_ini.sections)
				{
					if (s.name.empty())
						continue;
					auto hash = sh(s.name.c_str());
					ImageAtlas::Item item = { vec4(0.f, 0.f, 1.f, 1.f), vec4(0.f) };
					for (auto& e : s.entries)
					{
						if (e.key == "uvs")
							item.uvs = s2t<4, float>(e.values[0]);
						else if (e.key == "border")
							item.border = s2t<4, float>(e.values[0]);
					}
					ret->items[hash] = item;
				}

				ret->filename = filename;
				ret->ref = 1;
				loaded_image_atlases.emplace_back(ret);
				return ret;
			}
		}ImageAtlas_get;
		ImageAtlas::Get& ImageAtlas::get = ImageAtlas_get;

		struct ImageAtlasRelease : ImageAtlas::Release
		{
			void operator()(ImageAtlasPtr atlas) override
			{
				if (atlas->ref == 1)
				{
					std::erase_if(loaded_image_atlases, [&](const auto& i) {
						return i.get() == atlas;
					});
				}
				else
					atlas->ref--;
			}
		}ImageAtlas_release;
		ImageAtlas::Release& ImageAtlas::release = ImageAtlas_release;

		struct ImageAtlasGenerate : ImageAtlas::Generate
		{
			void operator()(const std::filesystem::path& folder) override
			{
				if (std::filesystem::exists(folder))
				{
					auto atlas_config_path = folder / L"atlas_config.ini";
					if (std::filesystem::exists(atlas_config_path))
					{
						uvec2 size(1024, 1024);
						auto atlas_config = parse_ini_file(atlas_config_path);
						for (auto& e : atlas_config.get_section_entries(""))
						{
							if (e.key == "size")
								size = s2t<2, uint>(e.values[0]);
						}

						auto atlas_path = folder / L".." / folder.filename();
						atlas_path += L".png";
						auto atlas_ini_path = atlas_path;
						atlas_ini_path += L".ini";
						auto atlas = Bitmap::create(size, 4);
						std::ofstream atlas_ini(atlas_ini_path);
						atlas_ini << "auto_mipmapping=true\n";

						std::unique_ptr<BinPackNode> bin_pack_root(new BinPackNode(size));
						for (auto& it : std::filesystem::directory_iterator(folder))
						{
							if (is_image_file(it.path().extension()))
							{
								if (auto bmp = Bitmap::create(it.path()); bmp)
								{
									ImageConfig config;
									auto has_config = read_image_config(it.path(), &config);

									if (auto n = bin_pack_root->find(ivec2(bmp->extent.x + 2, bmp->extent.y + 2)); n)
									{
										auto pos = n->pos + 1; // use a gap of 1, TODO: use a bigger gap to avoid bleeding while using mipmapping (But this will waste more space..)
										auto uv0 = vec2(pos.x, pos.y);
										auto uv1 = uv0 + vec2(bmp->extent.x, bmp->extent.y);
										bmp->copy_to(atlas, bmp->extent, ivec2(0), pos);
										atlas_ini << std::format("[{}]\n", it.path().filename().stem().string());
										atlas_ini << std::format("uvs={}\n", str(vec4(uv0 / (vec2)size, uv1 / (vec2)size)));
										if (has_config)
											atlas_ini << std::format("border={}\n", str(config.border));
									}
									delete bmp;
								}
							}
						}

						atlas->save(atlas_path);
						delete atlas;
					}
				}
			}
		}ImageAtlas_generate;
		ImageAtlas::Generate& ImageAtlas::generate = ImageAtlas_generate;
	}
}

