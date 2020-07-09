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
			switch (i->_format)
			{
			case Format_R8_UNORM:
				return 1;
			case Format_R16_UNORM:
				return 2;
			case Format_R32_SFLOAT:
				return 4;
			case Format_R8G8B8A8_UNORM: case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
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

		ImagePrivate::ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool default_view) :
			_d(d),
			_format(format),
			_size(size),
			_level(level),
			_layer(layer),
			_sample_count(sample_count)
		{
#if defined(FLAME_VULKAN)
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

			chk_res(vkCreateImage(d->vk_device, &imageInfo, nullptr, &_v));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->vk_device, _v, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = d->_find_memory_type(memRequirements.memoryTypeBits, MemPropDevice);

			chk_res(vkAllocateMemory(d->vk_device, &allocInfo, nullptr, &_m));

			chk_res(vkBindImageMemory(d->vk_device, _v, _m, 0));
#elif defined(FLAME_D3D12)

#endif
			if (default_view)
				_dv.reset(new ImageviewPrivate(this));

			if (data)
			{
				auto staging_buffer = std::make_unique<BufferPrivate>(d, image_pitch(get_pixel_size(this) * size.x()) * size.y(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->_map();
				memcpy(staging_buffer->_mapped, data, staging_buffer->_size);
				staging_buffer->_unmap();

				auto cb = std::make_unique<CommandBufferPrivate>(_d->_graphics_commandpool.get());
				cb->_begin(true);
				cb->_change_image_layout(this, ImageLayoutUndefined, ImageLayoutTransferDst);
				cb->_copy_buffer_to_image(staging_buffer.get(), this, { &BufferImageCopy(this->_size), 1 });
				cb->_change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				cb->_end();
				auto q = d->_graphics_queue.get();
				q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
				q->_wait_idle();
			}
		}

		ImagePrivate::ImagePrivate(DevicePrivate* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool default_view) :
			_d(d),
			_format(format),
			_size(size),
			_level(level),
			_layer(layer),
			_sample_count(SampleCount_1)
		{
#if defined(FLAME_VULKAN)
			_v = (VkImage)native;
			_m = 0;
#elif defined(FLAME_D3D12)
			_v = (ID3D12Resource*)native;
#endif

			_dv.reset(default_view ? new ImageviewPrivate(this) : nullptr);
		}

		ImagePrivate::~ImagePrivate()
		{
#if defined(FLAME_VULKAN)
			if (_m != 0)
			{
				vkFreeMemory(device->vk_device, _m, nullptr);
				vkDestroyImage(device->vk_device, _v, nullptr);
			}
#elif defined(FLAME_D3D12)

#endif
		}

		void ImagePrivate::_change_layout(ImageLayout from, ImageLayout to)
		{
			auto cb = std::make_unique<CommandBufferPrivate>(_d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(this, from, to);
			cb->_end();
			auto q = _d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();
		}

		void ImagePrivate::_clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color)
		{
			auto cb = std::make_unique<CommandBufferPrivate>(_d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(this, current_layout, ImageLayoutTransferDst);
			cb->_clear_image(this, color);
			cb->_change_image_layout(this, ImageLayoutTransferDst, after_layout);
			cb->_end();
			auto q = _d->_graphics_queue.get();
			q->_submit(std::array{ cb.get()}, nullptr, nullptr, nullptr);
			q->_wait_idle();
		}

		void ImagePrivate::_get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			assert(_format == Format_R8_UNORM || _format == Format_R8G8B8A8_UNORM || _format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(_d, data_size, BufferUsageTransferDst, MemPropHost);

			auto cb = std::make_unique<CommandBufferPrivate>(_d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			cb->_copy_image_to_buffer(this, stag_buf.get(), { &BufferImageCopy(Vec2u(extent), 0, 0, offset), 1 });
			cb->_change_image_layout(this, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
			cb->_end();
			auto q = _d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();

			stag_buf->_map();
			memcpy(dst, stag_buf->_mapped, stag_buf->_size);
			stag_buf->_flush();
		}

		void ImagePrivate::_set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			assert(_format == Format_R8_UNORM || _format == Format_R8G8B8A8_UNORM || _format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(_d, data_size, BufferUsageTransferSrc, MemPropHost);
			stag_buf->_map();
			memcpy(stag_buf->_mapped, src, stag_buf->_size);
			stag_buf->_flush();

			auto cb = std::make_unique<CommandBufferPrivate>(_d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->_copy_buffer_to_image(stag_buf.get(), this, { &BufferImageCopy(Vec2u(extent), 0, 0, offset), 1 });
			cb->_change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->_end();
			auto q = _d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();
		}

		ImagePrivate* ImagePrivate::_create(DevicePrivate* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool default_view)
		{
			auto i = new ImagePrivate(d, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()), Vec2u(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto staging_buffer = std::make_unique<BufferPrivate>(d, bmp->get_size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
			staging_buffer->_map();
			memcpy(staging_buffer->_mapped, bmp->get_data(), staging_buffer->_size);
			staging_buffer->_unmap();

			auto cb = std::make_unique<CommandBufferPrivate>(d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->_copy_buffer_to_image(staging_buffer.get(), i, { &BufferImageCopy(i->_size), 1 });
			cb->_change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->_end();
			auto q = d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();

			return i;
		}

		ImagePrivate* ImagePrivate::_create(DevicePrivate* d, const std::filesystem::path& filename, ImageUsageFlags extra_usage, bool default_view)
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

				//staging_buffer = create_buffer(d, gli_texture.size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
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

				staging_buffer.reset(new BufferPrivate(d, bmp->get_size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent));
				staging_buffer->_map();
				memcpy(staging_buffer->_mapped, bmp->get_data(), staging_buffer->_size);
				staging_buffer->_unmap();

				bmp->release();

				buffer_copy_regions.push_back(BufferImageCopy(Vec2u(width, height)));
			}

			auto i = new ImagePrivate(d, fmt, Vec2u(width, height), level, layer, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto cb = std::make_unique<CommandBufferPrivate>(d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->_copy_buffer_to_image(staging_buffer.get(), i, { buffer_copy_regions.data(), buffer_copy_regions.size() });
			cb->_change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->_end();
			auto q = d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();

			return i;
		}

		Image* Image::create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool default_view) { return new ImagePrivate((DevicePrivate*)d, format, size, level, layer, sample_count, usage, data, default_view); }
		Image* Image::create(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool create_default_view) { return ImagePrivate::_create((DevicePrivate*)d, bmp, extra_usage, create_default_view); }
		Image* Image::create(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage, bool create_defalut_view) { return ImagePrivate::_create((DevicePrivate*)d, filename, extra_usage, create_defalut_view); }

		ImageviewPrivate::ImageviewPrivate(ImagePrivate* image, ImageviewType type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle swizzle_r, Swizzle swizzle_g, Swizzle swizzle_b, Swizzle swizzle_a) :
			_image(image),
			_d(image->_d),
			_type(type),
			_base_level(base_level),
			_level_count(level_count),
			_base_layer(base_layer),
			_layer_count(layer_count),
			_swizzle_r(swizzle_r),
			_swizzle_g(swizzle_g),
			_swizzle_b(swizzle_b),
			_swizzle_a(swizzle_a)
		{
#if defined(FLAME_VULKAN)
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = to_backend(swizzle_r);
			info.components.g = to_backend(swizzle_g);
			info.components.b = to_backend(swizzle_b);
			info.components.a = to_backend(swizzle_a);
			info.image = image->_v;
			info.viewType = to_backend(type);
			info.format = to_backend(image->_format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspect>(aspect_from_format(image->_format));
			info.subresourceRange.baseMipLevel = base_level;
			info.subresourceRange.levelCount = level_count;
			info.subresourceRange.baseArrayLayer = base_layer;
			info.subresourceRange.layerCount = layer_count;

			chk_res(vkCreateImageView(device->vk_device, &info, nullptr, &_v));
#elif defined(FLAME_D3D12)
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			auto d = image->device->vk_device;
			auto res = d->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));

			//auto descriptor_size = d->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			auto descriptor_handle = v->GetCPUDescriptorHandleForHeapStart();
			d->CreateRenderTargetView(i->_v, nullptr, descriptor_handle);
			//descriptor_handle.ptr += descriptor_size;
#endif
		}

		ImageviewPrivate::~ImageviewPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyImageView(device->vk_device, _v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Imageview* Imageview::create(Image* image, ImageviewType type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle swizzle_r, Swizzle swizzle_g, Swizzle swizzle_b, Swizzle swizzle_a)
		{
			return new ImageviewPrivate((ImagePrivate*)image, type, base_level, level_count, base_layer, layer_count, swizzle_r, swizzle_g, swizzle_b, swizzle_a);
		}

		SamplerPrivate::SamplerPrivate(DevicePrivate* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates) :
			_d(d)
		{
#if defined(FLAME_VULKAN)
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

			chk_res(vkCreateSampler(device->vk_device, &info, nullptr, &_v));
#elif defined(FLAME_D3D12)

#endif
		}

		SamplerPrivate::~SamplerPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroySampler(device->vk_device, _v, nullptr);
#elif defined(FLAME_D3D12)

#endif
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
					_border = !(e.value == "0");
			}

			_image = ImagePrivate::_create(d, image_filename.c_str());

			auto w = (float)_image->_size.x();
			auto h = (float)_image->_size.y();

			for (auto& e : ini.get_section_entries("tiles"))
			{
				auto tile = new ImageTilePrivate;

				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile->_name = t;
				ss >> t;
				auto v = sto<Vec4u>(t.c_str());
				tile->_pos = Vec2i(v.x(), v.y());
				tile->_size = Vec2i(v.z(), v.w());
				tile->_uv.x() = tile->_pos.x() / w;
				tile->_uv.y() = tile->_pos.y() / h;
				tile->_uv.z() = (tile->_pos.x() + tile->_size.x()) / w;
				tile->_uv.w() = (tile->_pos.y() + tile->_size.y()) / h;

				_tiles.emplace_back(tile);
			}
		}

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			delete _image;
		}

		ImageTile* ImageAtlasPrivate::_find_tile(const std::string& name) const
		{
			for (auto& t : _tiles)
			{
				if (t->_name == name)
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

			auto atlas = new ImageAtlasPrivate((DevicePrivate*)d, filename);
			report_used_file(filename);

			return atlas;
		}
	}
}

