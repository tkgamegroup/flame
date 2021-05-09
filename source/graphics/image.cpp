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
		static uint get_pixel_size(ImagePtr i)
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

		std::vector<ImagePrivate*> __images;

		void ImagePrivate::build_sizes(const uvec2& size)
		{
			auto s = size;
			for (auto i = 0; i < levels; i++)
			{
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
			__images.push_back(this);

			build_sizes(size);

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
			__images.push_back(this);

			build_sizes(size);

			vk_image = (VkImage)native;

			build_default_views();
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

		void ImagePrivate::change_layout(ImageLayout src_layout, ImageLayout dst_layout)
		{
			InstanceCB cb(device);

			cb->image_barrier(this, { 0, levels, 0, layers }, src_layout, dst_layout);
		}

		void ImagePrivate::clear(ImageLayout src_layout, ImageLayout dst_layout, const cvec4& color)
		{
			InstanceCB cb(device);

			cb->image_barrier(this, { 0, levels, 0, layers }, src_layout, ImageLayoutTransferDst);
			cb->clear_color_image(this, color);
			cb->image_barrier(this, { 0, levels, 0, layers }, ImageLayoutTransferDst, dst_layout);
		}

		PipelinePtr sample_pipeline = nullptr;

		void ImagePrivate::get_samples(uint count, const vec2* uvs, vec4* dst)
		{
			if (!sample_pipeline)
			{
				sample_pipeline = PipelinePrivate::create(device, ShaderPrivate::get(device, L"image_sample/image_sample.comp"), 
					PipelineLayoutPrivate::get(device, L"image_sample/image_sample.pll"));
			}
			const auto MaxSamples = 1024 * 1024;
			if (!sample_uvs)
				sample_uvs.reset(new BufferPrivate(device, sizeof(vec2) * MaxSamples, BufferUsageStorage | BufferUsageTransferDst, MemoryPropertyDevice));
			if (!sample_res)
				sample_res.reset(new BufferPrivate(device, sizeof(vec4) * MaxSamples, BufferUsageStorage | BufferUsageTransferSrc, MemoryPropertyDevice));
			if (!sample_descriptorset)
			{
				auto dsl = DescriptorSetLayoutPrivate::get(device, L"image_sample/image_sample.dsl");
				sample_descriptorset.reset(new DescriptorSetPrivate(device->dsp.get(), dsl));
				sample_descriptorset->set_image(dsl->find_binding("tex"), 0, views[0].get(), SamplerPrivate::get(device, FilterLinear, FilterLinear, false, AddressClampToEdge));
				sample_descriptorset->set_buffer(dsl->find_binding("Samples"), 0, sample_uvs.get());
				sample_descriptorset->set_buffer(dsl->find_binding("Results"), 0, sample_res.get());
			}

			StagingBuffer stag(device, sizeof(vec4) * count, nullptr, BufferUsageTransferDst);

			{
				InstanceCB cb(device);

				BufferCopy cpy;
				cpy.size = sizeof(vec2) * count;
				memcpy(stag.mapped, uvs, cpy.size);
				cb->copy_buffer((BufferPrivate*)stag.get(), sample_uvs.get(), 1, &cpy);
				cb->buffer_barrier(sample_uvs.get(), AccessTransferWrite, AccessShaderRead);
				cb->bind_pipeline(sample_pipeline);
				cb->bind_descriptor_set(0, sample_descriptorset.get());
				cb->dispatch(vec3(count, 1, 1));
			}

			{
				InstanceCB cb(device);

				BufferCopy cpy;
				cpy.size = sizeof(vec4) * count;
				cb->copy_buffer(sample_res.get(), (BufferPrivate*)stag.get(), 1, &cpy);
			}

			memcpy(dst, stag.mapped, sizeof(vec4) * count);
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

			InstanceCB cb(device);

			cb->image_barrier(this, {}, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			cb->image_barrier(img, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
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

			std::swap(vk_image, img->vk_image);
			std::swap(vk_memory, img->vk_memory);
			delete img;
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

				StagingBuffer stag(device, gli_texture.size(), nullptr);
				std::vector<std::tuple<void*, void*, uint>> gli_cpies;
				{
					InstanceCB cb(device);
					std::vector<BufferImageCopy> cpies;
					auto dst = (char*)stag.mapped;
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
					cb->copy_image_to_buffer(this, (BufferPrivate*)stag.get(), cpies.size(), cpies.data());
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
				uvec2(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageStorage | ImageUsageTransferDst | ImageUsageTransferSrc);

			StagingBuffer stag(device, bmp->get_size(), bmp->get_data());
			InstanceCB cb(device);
			BufferImageCopy cpy;
			cpy.img_ext = i->sizes[0];
			cb->image_barrier(i, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image((BufferPrivate*)stag.get(), i, 1, &cpy);
			cb->image_barrier(i, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			return i;
		}

		ImagePrivate* ImagePrivate::get(DevicePrivate* device, const std::filesystem::path& filename, bool srgb)
		{
			auto& texs = device->texs[srgb ? 1 : 0];

			for (auto& tex : texs)
			{
				if (tex->filename == filename)
				{
					// TODO: add references
					return tex.get();
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
				auto gli_texture = gli::load(filename.string());

				auto ext = gli_texture.extent();
				auto levels = (uint)gli_texture.levels();
				auto layers = (uint)gli_texture.layers();

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

				ret = new ImagePrivate(device, format, ext, levels, layers,
					SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | ImageUsageTransferSrc, layers == 6);

				StagingBuffer stag(device, gli_texture.size(), nullptr);
				InstanceCB cb(device);
				std::vector<BufferImageCopy> cpies;
				auto dst = (char*)stag.mapped;
				auto offset = 0;
				for (auto i = 0; i < layers; i++)
				{
					for (auto j = 0; j < levels; j++)
					{
						auto size = gli_texture.size(j);
						auto ext = gli_texture.extent(j);
						memcpy(dst + offset, gli_texture.data(i, 0, j), size);

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
				cb->copy_buffer_to_image((BufferPrivate*)stag.get(), ret, cpies.size(), cpies.data());
				cb->image_barrier(ret, { 0, levels, 0, layers }, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			}
			else
			{
				UniPtr<Bitmap> bmp(Bitmap::create(filename.c_str()));
				if (srgb)
					bmp->srgb_to_linear();

				ret = create(device, bmp.get());
				ret->filename = filename;
			}

			texs.emplace_back(ret);

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

		ImageViewPrivate::ImageViewPrivate(ImagePrivate* image, bool auto_released, ImageViewType type, const ImageSub& sub, const ImageSwizzle& swizzle) :
			image(image),
			device(image->device),
			type(type),
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
			info.viewType = to_backend(type);
			info.format = to_backend(image->format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(image->format));
			info.subresourceRange.baseMipLevel = sub.base_level;
			info.subresourceRange.levelCount = sub.level_count;
			info.subresourceRange.baseArrayLayer = sub.base_layer;
			info.subresourceRange.layerCount = sub.layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &vk_image_view));

			if (auto_released)
				image->views.emplace_back(this);
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
		}

		ImageView* ImageView::create(Image* image, bool auto_released, ImageViewType type, const ImageSub& sub, const ImageSwizzle& swizzle)
		{
			return new ImageViewPrivate((ImagePrivate*)image, auto_released, type, sub, swizzle);
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

			image = ImagePrivate::get(device, image_filename.c_str(), false);

			auto w = (float)image->sizes[0].x;
			auto h = (float)image->sizes[0].y;

			for (auto& e : ini.get_section_entries("tiles"))
			{
				Tile tile;
				tile.index = tiles.size();
				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile.name = t;
				ss >> t;
				auto v = sto<uvec4>(t.c_str());
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

