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

		ImagePrivate::ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool _default_view) :
			device(d),
			format(format),
			size(size),
			level(level),
			layer(layer),
			sample_count(sample_count)
		{
			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = 0;
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

			chk_res(vkCreateImage(d->vk_device, &imageInfo, nullptr, &vk_image));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->vk_device, vk_image, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = d->find_memory_type(memRequirements.memoryTypeBits, MemoryPropertyDevice);

			chk_res(vkAllocateMemory(d->vk_device, &allocInfo, nullptr, &vk_memory));

			chk_res(vkBindImageMemory(d->vk_device, vk_image, vk_memory, 0));

			if (_default_view)
				default_view.reset(new ImageViewPrivate(this));

			if (data)
			{
				auto staging_buffer = std::make_unique<BufferPrivate>(d, image_pitch(get_pixel_size(this) * size.x()) * size.y(), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, data, staging_buffer->size);
				staging_buffer->unmap();

				auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
				cb->begin(true);
				cb->image_barrier(this, ImageLayoutUndefined, ImageLayoutTransferDst);
				BufferImageCopy cpy;
				cpy.image_extent = this->size;
				cb->copy_buffer_to_image(staging_buffer.get(), this, { &cpy, 1 });
				cb->image_barrier(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				cb->end();
				auto q = d->graphics_queue.get();
				q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->wait_idle();
			}
		}

		ImagePrivate::ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool _default_view) :
			device(d),
			format(format),
			size(size),
			level(level),
			layer(layer),
			sample_count(SampleCount_1)
		{
			vk_image = (VkImage)native;

			default_view.reset(_default_view ? new ImageViewPrivate(this) : nullptr);
		}

		ImagePrivate::~ImagePrivate()
		{
			if (vk_memory != 0)
			{
				vkFreeMemory(device->vk_device, vk_memory, nullptr);
				vkDestroyImage(device->vk_device, vk_image, nullptr);
			}
		}

		void ImagePrivate::change_layout(ImageLayout from, ImageLayout to)
		{
			auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(this, from, to);
			cb->end();
			auto q = device->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();
		}

		void ImagePrivate::clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color)
		{
			auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(this, current_layout, ImageLayoutTransferDst);
			cb->clear_image(this, color);
			cb->image_barrier(this, ImageLayoutTransferDst, after_layout);
			cb->end();
			auto q = device->graphics_queue.get();
			q->submit(std::array{ cb.get()}, nullptr, nullptr, nullptr);
			q->wait_idle();
		}

		void ImagePrivate::get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(device, data_size, BufferUsageTransferDst, MemoryPropertyHost);

			auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(this, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			BufferImageCopy cpy;
			cpy.image_offset = offset;
			cpy.image_extent = extent;
			cb->copy_image_to_buffer(this, stag_buf.get(), { &cpy, 1 });
			cb->image_barrier(this, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
			cb->end();
			auto q = device->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();

			stag_buf->map();
			memcpy(dst, stag_buf->mapped, stag_buf->size);
			stag_buf->flush();
		}

		void ImagePrivate::set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(device, data_size, BufferUsageTransferSrc, MemoryPropertyHost);
			stag_buf->map();
			memcpy(stag_buf->mapped, src, stag_buf->size);
			stag_buf->flush();

			auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(this, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			BufferImageCopy cpy;
			cpy.image_offset = offset;
			cpy.image_extent = extent;
			cb->copy_buffer_to_image(stag_buf.get(), this, { &cpy, 1 });
			cb->image_barrier(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			auto q = device->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();
		}

		ImagePrivate* ImagePrivate::create(DevicePrivate* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool default_view)
		{
			auto i = new ImagePrivate(d, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()), Vec2u(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto staging_buffer = std::make_unique<BufferPrivate>(d, bmp->get_size(), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent);
			staging_buffer->map();
			memcpy(staging_buffer->mapped, bmp->get_data(), staging_buffer->size);
			staging_buffer->unmap();

			auto cb = std::make_unique<CommandBufferPrivate>(d->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			BufferImageCopy cpy;
			cpy.image_offset = i->size;
			cb->copy_buffer_to_image(staging_buffer.get(), i, { &cpy, 1 });
			cb->image_barrier(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			auto q = d->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();

			return i;
		}

		ImagePrivate* ImagePrivate::create(DevicePrivate* d, const std::filesystem::path& filename, ImageUsageFlags extra_usage, bool default_view)
		{
			std::filesystem::path path(filename);
			if (!std::filesystem::exists(path))
			{
				wprintf(L"cannot find image: %s\n", filename);
				return nullptr;
			}

			int width, height, level, layer;
			auto fmt = Format_Undefined;

			std::unique_ptr<BufferPrivate> staging_buffer;
			std::vector<BufferImageCopy> buffer_copy_regions;

			auto ext = path.extension().string();
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
				auto channel = bmp->get_channel();
				if (channel == 3)
					bmp->add_alpha_channel();

				width = bmp->get_width();
				height = bmp->get_height();
				level = layer = 1;

				fmt = get_image_format(channel, bmp->get_byte_per_channel());

				staging_buffer.reset(new BufferPrivate(d, bmp->get_size(), BufferUsageTransferSrc, MemoryPropertyHost | MemoryPropertyCoherent));
				staging_buffer->map();
				memcpy(staging_buffer->mapped, bmp->get_data(), staging_buffer->size);
				staging_buffer->unmap();

				bmp->release();

				BufferImageCopy cpy;
				cpy.image_extent = Vec2u(width, height);
				buffer_copy_regions.push_back(cpy);
			}

			auto i = new ImagePrivate(d, fmt, Vec2u(width, height), level, layer, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto cb = std::make_unique<CommandBufferPrivate>(d->graphics_command_pool.get());
			cb->begin(true);
			cb->image_barrier(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer.get(), i, { buffer_copy_regions.data(), buffer_copy_regions.size() });
			cb->image_barrier(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			auto q = d->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();

			return i;
		}

		Image* Image::create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool default_view) { return new ImagePrivate((DevicePrivate*)d, format, size, level, layer, sample_count, usage, data, default_view); }
		Image* Image::create(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool create_default_view) { return ImagePrivate::create((DevicePrivate*)d, bmp, extra_usage, create_default_view); }
		Image* Image::create(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage, bool create_defalut_view) { return ImagePrivate::create((DevicePrivate*)d, filename, extra_usage, create_defalut_view); }

		ImageViewPrivate::ImageViewPrivate(ImagePrivate* image, ImageViewType type, const ImageSubresource& subresource, const ImageSwizzle& swizzle) :
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
		}

		ImageViewPrivate::~ImageViewPrivate()
		{
			vkDestroyImageView(device->vk_device, vk_image_view, nullptr);
		}

		ImageView* ImageView::create(Image* image, ImageViewType type, const ImageSubresource& subresource, const ImageSwizzle& swizzle)
		{
			return new ImageViewPrivate((ImagePrivate*)image, type, subresource, swizzle);
		}

		SamplerPrivate::SamplerPrivate(DevicePrivate* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates) :
			device(d)
		{
			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.magFilter = to_backend(mag_filter);
			info.minFilter = to_backend(min_filter);
			info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			info.anisotropyEnable = VK_FALSE;
			info.maxAnisotropy = 1.f;
			info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
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

		Sampler* Sampler::create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			return new SamplerPrivate((DevicePrivate*)d, mag_filter, min_filter, unnormalized_coordinates);
		}

		ImageAtlasPrivate::ImageAtlasPrivate(DevicePrivate* d, const std::wstring& filename)
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

			image = ImagePrivate::create(d, image_filename.c_str());

			auto w = (float)image->size.x();
			auto h = (float)image->size.y();

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

		ImageAtlas* ImageAtlas::create(Device* d, const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find atlas: %s\n", filename);
				return nullptr;
			}

			return new ImageAtlasPrivate((DevicePrivate*)d, filename);
		}
	}
}

