#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
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
		static uint get_pixel_size(ImagePrivate* i)
		{
			switch (i->format)
			{
			case Format_R8_UNORM:
				return 1;
			case Format_R16_UNORM:
				return 2;
			case Format_R32_SFLOAT:
				return 4;
			case Format_R8G8B8A8_UNORM: case Format_B8G8R8A8_UNORM:
				return 4;
			case Format_R16G16B16A16_UNORM: case Format_R16G16B16A16_SFLOAT:
				return 8;
			case Format_R32G32B32A32_SFLOAT:
				return 16;
			case Format_Depth16:
				return 2;
			}
			return 0;
		}

		void ImagePrivate::init(const uvec2& size)
		{
			auto s = size;
			if (levels == 0)
				levels = 100;
			for (auto i = 0; i < levels; i++)
			{
				if (s.x == 0 && s.y == 0)
				{
					levels = i;
					break;
				}
				sizes.push_back(max(s, uvec2(1U)));
				s.x >>= 1;
				s.y >>= 1;
			}
		}

		void ImagePrivate::build_default_views()
		{
			views.resize(levels);
			for (auto i = 0; i < levels; i++)
				views[i].reset(new ImageViewPrivate(this, false, ImageView2D, { (uint)i }));
			if (levels > 1 || layers > 1)
				views.emplace_back(new ImageViewPrivate(this, false, !is_cube ? ImageView2D : ImageViewCube, { 0U, levels, 0U, layers }));
		}

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, SampleCount sample_count, ImageUsageFlags usage, bool is_cube) :
			device(device),
			format(format),
			levels(levels),
			layers(layers),
			sample_count(sample_count),
			usage(usage),
			is_cube(is_cube)
		{
			init(size);
			levels = ImagePrivate::levels;

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

			build_default_views();
		}

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const uvec2& size, uint levels, uint layers, void* native) :
			device(device),
			format(format),
			levels(levels),
			layers(layers),
			sample_count(SampleCount_1),
			usage(usage)
		{
			init(size);

			vk_image = (VkImage)native;

			build_default_views();
		}

		ImagePrivate::~ImagePrivate()
		{
			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
			}
		}

		void ImagePrivate::clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color)
		{
			ImmediateCommandBuffer icb(device);
			auto cb = icb.cb.get();

			cb->image_barrier(this, {}, src_layout, ImageLayoutTransferDst);
			cb->clear_color_image(this, color);
			cb->image_barrier(this, {}, ImageLayoutTransferDst, dst_layout);
		}

		PipelinePrivate* sample_pipeline = nullptr;

		void ImagePrivate::get_samples(uint count, const vec2* uvs, vec4* dst)
		{
			if (!sample_pipeline)
			{
				sample_pipeline = PipelinePrivate::create(device, ShaderPrivate::get(device, L"image_sample/image_sample.comp"), 
					PipelineLayoutPrivate::get(device, L"image_sample/image_sample.pll"));
			}
			if (!sample_uvs)
				sample_uvs.reset(new BufferPrivate(device, sizeof(vec2) * 1024 * 1024, BufferUsageStorage | BufferUsageTransferDst, MemoryPropertyDevice));
			if (!sample_res)
				sample_res.reset(new BufferPrivate(device, sizeof(vec4) * 1024 * 1024, BufferUsageStorage | BufferUsageTransferSrc, MemoryPropertyDevice));
			if (!sample_descriptorset)
			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"image_sample/image_sample.dsl");
				sample_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				sample_descriptorset->set_image(dsl->find_binding("tex"), 0, views[0].get(), SamplerPrivate::get(device, FilterLinear, FilterLinear, false, AddressClampToEdge));
				sample_descriptorset->set_buffer(dsl->find_binding("Samples"), 0, sample_uvs.get());
				sample_descriptorset->set_buffer(dsl->find_binding("Results"), 0, sample_res.get());
			}

			ImmediateStagingBuffer stag(device, sizeof(vec4) * count, nullptr, BufferUsageTransferDst);

			{
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();
				BufferCopy cpy;
				cpy.size = sizeof(vec2) * count;
				memcpy(stag.buf->mapped, uvs, cpy.size);
				cb->copy_buffer(stag.buf.get(), sample_uvs.get(), { &cpy, 1 });
				cb->buffer_barrier(sample_uvs.get(), AccessTransferWrite, AccessShaderRead);
				cb->bind_pipeline(sample_pipeline);
				cb->bind_descriptor_set(sample_descriptorset.get(), 0);
				cb->dispatch(vec3(count, 1, 1));
			}

			{
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();
				BufferCopy cpy;
				cpy.size = sizeof(vec4) * count;
				cb->copy_buffer(sample_res.get(), stag.buf.get(), { &cpy, 1 });
			}

			memcpy(dst, stag.buf->mapped, sizeof(vec4) * count);
		}

		void ImagePrivate::save(const std::filesystem::path& filename)
		{
			fassert(usage & ImageUsageTransferSrc);

			auto ext = filename.extension();
			if (ext == L".ktx" || ext == L".dds")
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

				ImmediateStagingBuffer stag(device, gli_texture.size(), nullptr);
				std::vector<std::tuple<void*, void*, uint>> gli_cpies;
				{
					ImmediateCommandBuffer icb(device);
					auto cb = icb.cb.get();
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)stag.buf->mapped;
					auto offset = 0;
					for (auto i = 0; i < layers; i++)
					{
						for (auto j = 0; j < levels; j++)
						{
							auto size = (uint)gli_texture.size(j);
							
							gli_cpies.emplace_back(gli_texture.data(i, 0, j), dst + offset, size);

							BufferImageCopy cpy;
							cpy.buffer_offset = offset;
							auto ext = gli_texture.extent(j);
							cpy.image_extent = uvec2(ext.x, ext.y);
							cpy.image_level = j;
							cpy.image_base_layer = i;
							cpies.push_back(cpy);

							offset += size;
						}
					}
					cb->image_barrier(this, {}, ImageLayoutUndefined, ImageLayoutTransferSrc);
					cb->copy_image_to_buffer(this, stag.buf.get(), cpies);
					cb->image_barrier(this, {}, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
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
				uvec2(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageStorage | ImageUsageTransferDst);

			ImmediateStagingBuffer stag(device, bmp->get_size(), bmp->get_data());
			ImmediateCommandBuffer icb(device);
			auto cb = icb.cb.get();
			BufferImageCopy cpy;
			cpy.image_extent = i->sizes[0];
			cb->image_barrier(i, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(stag.buf.get(), i, { &cpy, 1 });
			cb->image_barrier(i, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			return i;
		}

		ImagePrivate* ImagePrivate::create(DevicePrivate* device, const std::filesystem::path& filename, bool srgb, ImageUsageFlags additional_usage, bool is_cube, bool generate_mipmaps)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find image: %s\n", filename.c_str());
				return nullptr;
			}

			ImagePrivate* ret = nullptr;

			if (generate_mipmaps)
				additional_usage = additional_usage | ImageUsageTransferSrc;

			auto ext = filename.extension();
			if (ext == L".ktx" || ext == L".dds")
			{
				auto gli_texture = gli::load(filename.string());

				auto ext = gli_texture.extent();
				auto levels = gli_texture.levels();
				auto layers = gli_texture.layers();

				fassert(!(generate_mipmaps && levels != 1));

				Format format = Format_Undefined;
				switch (gli_texture.format())
				{
				case gli::FORMAT_RGBA8_UNORM_PACK8:
					format = Format_R8G8B8A8_UNORM;
					break;
				case gli::FORMAT_RGBA16_SFLOAT_PACK16:
					format = Format_R16G16B16A16_SFLOAT;
					break;
				}
				fassert(format != Format_Undefined);

				ret = new ImagePrivate(device, format, ext, generate_mipmaps ? 0 : levels, layers,
					SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | additional_usage, is_cube);

				ImmediateStagingBuffer stag(device, gli_texture.size(), nullptr);
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();
				std::vector<BufferImageCopy> cpies;
				auto dst = (char*)stag.buf->mapped;
				auto offset = 0;
				for (auto i = 0; i < layers; i++)
				{
					for (auto j = 0; j < levels; j++)
					{
						auto size = gli_texture.size(j);
						auto ext = gli_texture.extent(j);
						memcpy(dst + offset, gli_texture.data(i, 0, j), size);

						BufferImageCopy cpy;
						cpy.buffer_offset = offset;
						cpy.image_extent = ext;
						cpy.image_level = j;
						cpy.image_base_layer = i;
						cpies.push_back(cpy);

						offset += size;
					}
				}
				cb->image_barrier(ret, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
				cb->copy_buffer_to_image(stag.buf.get(), ret, cpies);
				cb->image_barrier(ret, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}
			else
			{
				FlmPtr<Bitmap> bmp(Bitmap::create(filename.c_str()));
				if (srgb)
					bmp->srgb_to_linear();

				ret = new ImagePrivate(device, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()), uvec2(bmp->get_width(), bmp->get_height()), 
					generate_mipmaps ? 0 : 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | additional_usage);
				ret->filename = filename;

				ImmediateStagingBuffer stag(device, bmp->get_size(), bmp->get_data());
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();
				BufferImageCopy cpy;
				cpy.image_extent = ret->sizes[0];
				cb->image_barrier(ret, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
				cb->copy_buffer_to_image(stag.buf.get(), ret, { &cpy, 1 });
				cb->image_barrier(ret, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}

			if (generate_mipmaps)
			{
				ImmediateCommandBuffer icb(device);
				auto cb = icb.cb.get();

				for (auto i = 1U; i < ret->levels; i++) 
				{
					cb->image_barrier(ret, { i - 1, 1, 0, 1 }, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc, AccessShaderRead, AccessTransferRead);

					auto s1 = (ivec2)ret->sizes[i - 1];
					auto s2 = (ivec2)ret->sizes[i];

					VkImageBlit blit{};
					blit.srcOffsets[0] = { 0, 0, 0 };
					blit.srcOffsets[1] = { s1.x, s1.y, 1 };
					blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.srcSubresource.mipLevel = i - 1;
					blit.srcSubresource.baseArrayLayer = 0;
					blit.srcSubresource.layerCount = 1;
					blit.dstOffsets[0] = { 0, 0, 0 };
					blit.dstOffsets[1] = { s2.x, s2.y, 1 };
					blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					blit.dstSubresource.mipLevel = i;
					blit.dstSubresource.baseArrayLayer = 0;
					blit.dstSubresource.layerCount = 1;

					vkCmdBlitImage(cb->vk_command_buffer, ret->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
						ret->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

					cb->image_barrier(ret, { i - 1, 1, 0, 1 }, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly, AccessTransferRead, AccessShaderRead);
				}

				cb->image_barrier(ret, { ret->levels - 1, 1, 0, 1 }, ImageLayoutTransferDst, ImageLayoutShaderReadOnly, AccessTransferWrite, AccessShaderRead);
			}

			return ret;
		}

		Image* Image::create(Device* device, Format format, const uvec2& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool is_cube) 
		{ 
			return new ImagePrivate((DevicePrivate*)device, format, size, level, layer, sample_count, usage, is_cube);
		}

		Image* Image::create(Device* device, Bitmap* bmp) { return ImagePrivate::create((DevicePrivate*)device, bmp); }

		Image* Image::create(Device* device, const wchar_t* filename, bool srgb, ImageUsageFlags additional_usage, bool is_cube, bool generate_mipmaps) { return ImagePrivate::create((DevicePrivate*)device, filename, srgb, additional_usage, is_cube, generate_mipmaps); }

		ImageViewPrivate::ImageViewPrivate(ImagePrivate* image, bool auto_released, ImageViewType type, const ImageSubresource& subresource, const ImageSwizzle& swizzle) :
			image(image),
			device(image->device),
			type(type),
			subresource(subresource),
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
			info.viewType = to_backend(type);
			info.format = to_backend(image->format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(image->format));
			info.subresourceRange.baseMipLevel = subresource.base_level;
			info.subresourceRange.levelCount = subresource.level_count;
			info.subresourceRange.baseArrayLayer = subresource.base_layer;
			info.subresourceRange.layerCount = subresource.layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &vk_image_view));

			if (auto_released)
				image->views.emplace_back(this);
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
		}

		ImageView* ImageView::create(Image* image, bool auto_released, ImageViewType type, const ImageSubresource& subresource, const ImageSwizzle& swizzle)
		{
			return new ImageViewPrivate((ImagePrivate*)image, auto_released, type, subresource, swizzle);
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
			info.addressModeU = info.addressModeV = info.addressModeW =
				to_backend(address_mode);
			info.maxAnisotropy = 1.f;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;

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

		ImageAtlasPrivate::ImageAtlasPrivate(DevicePrivate* device, const std::wstring& filename)
		{
			std::wstring image_filename;
			auto ini = parse_ini_file(filename);
			for (auto& e : ini.get_section_entries(""))
			{
				if (e.key == "image")
					image_filename = std::filesystem::path(filename).parent_path() / e.value;
				else if (e.key == "border")
					border = !(e.value == "0");
			}

			image = ImagePrivate::create(device, image_filename.c_str(), false);

			auto w = (float)image->sizes[0].x;
			auto h = (float)image->sizes[0].y;

			for (auto& e : ini.get_section_entries("tiles"))
			{
				auto tile = new ImageTilePrivate;
				tile->index = tiles.size();
				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile->name = t;
				ss >> t;
				auto v = sto<uvec4>(t.c_str());
				tile->pos = ivec2(v.x, v.y);
				tile->size = ivec2(v.z, v.w);
				tile->uv.x = tile->pos.x / w;
				tile->uv.y = tile->pos.y / h;
				tile->uv.z = (tile->pos.x + tile->size.x) / w;
				tile->uv.w = (tile->pos.y + tile->size.y) / h;

				tiles.emplace_back(tile);
			}
		}

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			delete image;
		}

		ImageTilePrivate* ImageAtlasPrivate::find_tile(const std::string& name) const
		{
			for (auto& t : tiles)
			{
				if (t->name == name)
					return t.get();
			}
			return nullptr;;
		}

		ImageAtlas* ImageAtlas::create(Device* device, const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find atlas: %s\n", filename);
				return nullptr;
			}

			return new ImageAtlasPrivate((DevicePrivate*)device, filename);
		}
	}
}

