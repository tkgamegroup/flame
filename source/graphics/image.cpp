#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "command_private.h"
#include "shader_private.h"

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

		void ImagePrivate::init(const Vec2u& size)
		{
			auto s = size;
			for (auto i = 0; i < level; i++)
			{
				if (s.x() == 0 && s.y() == 0)
				{
					level = i;
					break;
				}
				sizes.push_back(max(s, Vec2u(1U)));
				s.x() >>= 1;
				s.y() >>= 1;
			}
		}

		void ImagePrivate::build_default_views()
		{
			views.resize(level);
			for (auto i = 0; i < level; i++)
				views[i].reset(new ImageViewPrivate(this, false, ImageView2D, { (uint)i }));
			if (level > 1)
				views.emplace_back(new ImageViewPrivate(this, false, ImageView2D, { (uint)0, (uint)level }));
		}

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const Vec2u& size, uint _level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool is_cube) :
			device(device),
			format(format),
			level(_level),
			layer(layer),
			sample_count(sample_count)
		{
			init(size);

			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = is_cube ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
			imageInfo.pNext = nullptr;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = to_backend(format);
			imageInfo.extent.width = size.x();
			imageInfo.extent.height = size.y();
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = level;
			imageInfo.arrayLayers = layer;
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

		ImagePrivate::ImagePrivate(DevicePrivate* device, Format format, const Vec2u& size, uint _level, uint layer, void* native) :
			device(device),
			format(format),
			level(_level),
			layer(layer),
			sample_count(SampleCount_1)
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

		ImagePrivate* ImagePrivate::create(DevicePrivate* device, Bitmap* bmp)
		{
			auto i = new ImagePrivate(device, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()),
				Vec2u(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageStorage | ImageUsageTransferDst);

			ImmediateStagingBuffer stag(bmp->get_size(), bmp->get_data());
			ImmediateCommandBuffer icb;
			auto cb = icb.cb.get();
			BufferImageCopy cpy;
			cpy.image_extent = i->sizes[0];
			cb->image_barrier(i, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(stag.buf.get(), i, { &cpy, 1 });
			cb->image_barrier(i, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

			return i;
		}

		ImagePrivate* ImagePrivate::create(DevicePrivate* device, const std::filesystem::path& filename, bool srgb, ImageUsageFlags additional_usage)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find image: %s\n", filename.c_str());
				return nullptr;
			}

			ImagePrivate* ret = nullptr;

			auto ext = filename.extension().string();
			if (ext == ".ktx" || ext == ".dds")
			{
				//gli::gl GL(gli::gl::PROFILE_GL33);

				//auto gli_texture = gli::load(filename);
				//if (gli_texture.empty())
				//	assert(0);

				//assert(gli_texture.target() == gli::TARGET_2D);
				//auto const gli_format = GL.translate(gli_texture.format(), gli_texture.swizzles());

				//width = gli_texture.extent().x();
				//height = gli_texture.extent().y();
				//level = gli_texture.levels();
				//layer = gli_texture.layers();

				//switch (gli_format.Internal)
				//{
				//case gli::gl::INTERNAL_RGBA_DXT5:
				//	fmt = Format_RGBA_BC3;
				//	break;
				//case gli::gl::INTERNAL_RGBA_ETC2:
				//	fmt = Format_RGBA_ETC2;
				//	break;
				//}

				//staging_buffer = create_buffer(d, gli_texture.size(), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent);
				//staging_buffer->_map();
				//memcpy(staging_buffer->_mapped, gli_texture.data(), staging_buffer->_size);
				//staging_buffer->_unmap();

				//auto offset override;
				//for (auto i override; i < level; i++)
				//{
				//	BufferTextureCopy c;
				//	c.buffer_offset = offset;
				//	c.image_x override;
				//	c.image_y override;
				//	c.image_width = gli_texture.extent(i).x();
				//	c.image_height = gli_texture.extent(i).y();
				//	c.image_level = i;
				//	buffer_copy_regions.push_back(c);
				//	offset += gli_texture.size(i);
				//}
			}
			else
			{
				auto bmp = Bitmap::create(filename.c_str());
				if (bmp->get_channel() == 3)
					bmp->add_alpha_channel();
				if (srgb)
					bmp->srgb_to_linear();

				ret = new ImagePrivate(device, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()), Vec2u(bmp->get_width(), bmp->get_height()), 1, 1,
					SampleCount_1, ImageUsageSampled | ImageUsageStorage | ImageUsageTransferDst | additional_usage);
				ret->filename = filename;

				ImmediateStagingBuffer stag(bmp->get_size(), bmp->get_data());
				ImmediateCommandBuffer icb;
				auto cb = icb.cb.get();
				BufferImageCopy cpy;
				cpy.image_extent = ret->sizes[0];
				cb->image_barrier(ret, {}, ImageLayoutUndefined, ImageLayoutTransferDst);
				cb->copy_buffer_to_image(stag.buf.get(), ret, { &cpy, 1 });
				cb->image_barrier(ret, {}, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);

				bmp->release();
			}

			return ret;
		}

		Image* Image::create(Device* device, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, bool is_cube) 
		{ 
			return new ImagePrivate((DevicePrivate*)device, format, size, level, layer, sample_count, usage, is_cube);
		}
		Image* Image::create(Device* device, Bitmap* bmp) { return ImagePrivate::create((DevicePrivate*)device, bmp); }
		Image* Image::create(Device* device, const wchar_t* filename, bool srgb, ImageUsageFlags additional_usage) { return ImagePrivate::create((DevicePrivate*)device, filename, srgb, additional_usage); }

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

		SamplerPrivate::SamplerPrivate(DevicePrivate* device, Filter mag_filter, Filter min_filter, AddressMode address_mode, bool unnormalized_coordinates) :
			device(device)
		{
			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.magFilter = to_backend(mag_filter);
			info.minFilter = to_backend(min_filter);
			auto vk_address_mode = to_backend(address_mode);
			info.addressModeU = vk_address_mode;
			info.addressModeV = vk_address_mode;
			info.addressModeW = vk_address_mode;
			info.anisotropyEnable = VK_FALSE;
			info.maxAnisotropy = 1.f;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			info.unnormalizedCoordinates = unnormalized_coordinates;
			info.compareEnable = VK_FALSE;
			info.compareOp = VK_COMPARE_OP_ALWAYS;
			info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			info.mipLodBias = 0.f;
			info.minLod = 0.f;
			info.maxLod = 0.f;

			chk_res(vkCreateSampler(device->vk_device, &info, nullptr, &vk_sampler));
		}

		SamplerPrivate::~SamplerPrivate()
		{
			vkDestroySampler(device->vk_device, vk_sampler, nullptr);
		}

		Sampler* Sampler::create(Device* device, Filter mag_filter, Filter min_filter, AddressMode address_mode, bool unnormalized_coordinates)
		{
			return new SamplerPrivate((DevicePrivate*)device, mag_filter, min_filter, address_mode, unnormalized_coordinates);
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

			auto w = (float)image->sizes[0].x();
			auto h = (float)image->sizes[0].y();

			for (auto& e : ini.get_section_entries("tiles"))
			{
				auto tile = new ImageTilePrivate;
				tile->index = tiles.size();
				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile->name = t;
				ss >> t;
				auto v = sto<Vec4u>(t.c_str());
				tile->pos = Vec2i(v.x(), v.y());
				tile->size = Vec2i(v.z(), v.w());
				tile->uv.x() = tile->pos.x() / w;
				tile->uv.y() = tile->pos.y() / h;
				tile->uv.z() = (tile->pos.x() + tile->size.x()) / w;
				tile->uv.w() = (tile->pos.y() + tile->size.y()) / h;

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

