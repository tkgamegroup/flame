#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include "device_private.h"
#include "renderpass_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "commandbuffer_private.h"
#include "shader_private.h"

namespace flame
{
	namespace graphics
	{
		ImagePrivate::ImagePrivate(DevicePrivate* d, Format _format, const Vec2u& _size, uint _level, uint _layer, SampleCount _sample_count, ImageUsageFlags usage, bool default_view) :
			d(d)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = _sample_count;

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

			chk_res(vkCreateImage(d->v, &imageInfo, nullptr, &v));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->v, v, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = d->find_memory_type(memRequirements.memoryTypeBits, MemPropDevice);

			chk_res(vkAllocateMemory(d->v, &allocInfo, nullptr, &m));

			chk_res(vkBindImageMemory(d->v, v, m, 0));
#elif defined(FLAME_D3D12)

#endif
			dv.reset(default_view ? new ImageviewPrivate(this) : nullptr);
		}

		ImagePrivate::ImagePrivate(DevicePrivate* d, Format _format, const Vec2u& _size, uint _level, uint _layer, void* native, bool default_view) :
			d(d)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = SampleCount_1;

#if defined(FLAME_VULKAN)
			v = (VkImage)native;
			m = 0;
#elif defined(FLAME_D3D12)
			v = (ID3D12Resource*)native;
#endif

			dv.reset(default_view ? new ImageviewPrivate(this) : nullptr);
		}

		ImagePrivate::~ImagePrivate()
		{
#if defined(FLAME_VULKAN)
			if (m != 0)
			{
				vkFreeMemory(d->v, m, nullptr);
				vkDestroyImage(d->v, v, nullptr);
			}
#elif defined(FLAME_D3D12)

#endif
		}

		void ImagePrivate::release() { delete this; }

		Format ImagePrivate::get_format() const { return format; }
		Vec2u ImagePrivate::get_size() const { return size; }
		uint ImagePrivate::get_level() const { return level; }
		uint ImagePrivate::get_layer() const { return layer; }
		SampleCount ImagePrivate::get_sample_count() const { return sample_count; }

		Imageview* ImagePrivate::get_default_view() const { return dv.get(); }

		uint get_pixel_size(ImagePrivate* i)
		{
			switch (i->format)
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

		void ImagePrivate::change_layout(ImageLayout from, ImageLayout to)
		{
			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(this, from, to);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);
		}

		void ImagePrivate::clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color)
		{
			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(this, current_layout, ImageLayoutTransferDst);
			cb->clear_image(this, color);
			cb->change_image_layout(this, ImageLayoutTransferDst, after_layout);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);
		}

		void ImagePrivate::get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(d, data_size, BufferUsageTransferDst, MemPropHost);

			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(this, stag_buf.get(), 1, &BufferImageCopy(Vec2u(extent), 0, 0, offset));
			cb->change_image_layout(this, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);

			stag_buf->map();
			memcpy(dst, stag_buf->mapped, stag_buf->size);
			stag_buf->flush();
		}

		void ImagePrivate::set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = image_pitch(get_pixel_size(this) * extent.x()) * extent.y();

			auto stag_buf = std::make_unique<BufferPrivate>(d, data_size, BufferUsageTransferSrc, MemPropHost);
			stag_buf->map();
			memcpy(stag_buf->mapped, src, stag_buf->size);
			stag_buf->flush();

			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(stag_buf.get(), this, 1, &BufferImageCopy(Vec2u(extent), 0, 0, offset));
			cb->change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);
		}

		Image* Image::create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool default_view)
		{
			auto i = new ImagePrivate((DevicePrivate*)d, format, size, level, layer, sample_count, usage);

			if (data)
			{
				auto staging_buffer = std::make_unique<BufferPrivate>((DevicePrivate*)d, image_pitch(get_pixel_size(i) * size.x()) * size.y(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, data, staging_buffer->size);
				staging_buffer->unmap();

				auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
				cb->begin(true);
				cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
				BufferImageCopy copy(i->size);
				cb->copy_buffer_to_image(staging_buffer.get(), i, 1, &copy);
				cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				cb->end();
				Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
				Queue::get_default(QueueGraphics)->wait_idle();
				Commandbuffer::destroy(cb);
			}

			return i;
		}

		Image* Image::create(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool default_view)
		{
			auto i = new ImagePrivate((DevicePrivate*)d, get_image_format(bmp->get_channel(), bmp->get_byte_per_channel()), Vec2u(bmp->get_width(), bmp->get_height()), 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto staging_buffer = std::make_unique<BufferPrivate>((DevicePrivate*)d, bmp->get_size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
			staging_buffer->map();
			memcpy(staging_buffer->mapped, bmp->get_data(), staging_buffer->size);
			staging_buffer->unmap();

			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			BufferImageCopy copy(i->size);
			cb->copy_buffer_to_image(staging_buffer.get(), i, 1, &copy);
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);

			return i;
		}

		Image* Image::create(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage, bool default_view)
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
				//staging_buffer->map();
				//memcpy(staging_buffer->mapped, gli_texture.data(), staging_buffer->size);
				//staging_buffer->unmap();

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
				auto bmp = Bitmap::create(filename);
				auto channel = bmp->get_channel();
				if (channel == 3)
					bmp->add_alpha_channel();

				width = bmp->get_width();
				height = bmp->get_height();
				level = layer = 1;

				fmt = get_image_format(channel, bmp->get_byte_per_channel());

				staging_buffer.reset(new BufferPrivate((DevicePrivate*)d, bmp->get_size(), BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent));
				staging_buffer->map();
				memcpy(staging_buffer->mapped, bmp->get_data(), staging_buffer->size);
				staging_buffer->unmap();

				bmp->release();

				buffer_copy_regions.push_back(BufferImageCopy(Vec2u(width, height)));
			}

			auto i = Image::create(d, fmt, Vec2u(width, height), level, layer, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer.get(), i, buffer_copy_regions.size(), buffer_copy_regions.data());
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			Queue::get_default(QueueGraphics)->submit(1, &cb, nullptr, nullptr, nullptr);
			Queue::get_default(QueueGraphics)->wait_idle();
			Commandbuffer::destroy(cb);

			return i;
		}

		ImageviewPrivate::ImageviewPrivate(Image* _image, ImageviewType _type, uint _base_level, uint _level_count, uint _base_layer, uint _layer_count, Swizzle _swizzle_r, Swizzle _swizzle_g, Swizzle _swizzle_b, Swizzle _swizzle_a) :
			image((ImagePrivate*)_image)
		{
			d = image->d;
			type = _type;
			base_level = _base_level;
			level_count = _level_count;
			base_layer = _base_layer;
			layer_count = _layer_count;

			swizzle_r = _swizzle_r;
			swizzle_g = _swizzle_g;
			swizzle_b = _swizzle_b;
			swizzle_a = _swizzle_a;

#if defined(FLAME_VULKAN)
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = to_backend(swizzle_r);
			info.components.g = to_backend(swizzle_g);
			info.components.b = to_backend(swizzle_b);
			info.components.a = to_backend(swizzle_a);
			info.image = image->v;
			info.viewType = to_backend(type);
			info.format = to_backend(image->format);
			info.subresourceRange.aspectMask = to_backend_flags<ImageAspect>(aspect_from_format(image->format));
			info.subresourceRange.baseMipLevel = base_level;
			info.subresourceRange.levelCount = level_count;
			info.subresourceRange.baseArrayLayer = base_layer;
			info.subresourceRange.layerCount = layer_count;

			chk_res(vkCreateImageView(d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			auto d = image->d->v;
			auto res = d->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));

			//auto descriptor_size = d->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
			auto descriptor_handle = v->GetCPUDescriptorHandleForHeapStart();
			d->CreateRenderTargetView(i->v, nullptr, descriptor_handle);
			//descriptor_handle.ptr += descriptor_size;
#endif
		}

		ImageviewPrivate::~ImageviewPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyImageView(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void ImageviewPrivate::release() { delete this; }

		Image* ImageviewPrivate::get_image() const { return image; }

		ImageviewType ImageviewPrivate::get_type() const { return type; }
		uint ImageviewPrivate::get_base_level() const { return base_level; }
		uint ImageviewPrivate::get_level_count() const { return level_count; }
		uint ImageviewPrivate::get_base_layer() const { return base_layer; }
		uint ImageviewPrivate::get_layer_count() const { return layer_count; }
		Swizzle ImageviewPrivate::get_swizzle_r() const { return swizzle_r; }
		Swizzle ImageviewPrivate::get_swizzle_g() const { return swizzle_g; }
		Swizzle ImageviewPrivate::get_swizzle_b() const { return swizzle_b; }
		Swizzle ImageviewPrivate::get_swizzle_a() const { return swizzle_a; }

		Imageview* Imageview::create(Image* image, ImageviewType type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle swizzle_r, Swizzle swizzle_g, Swizzle swizzle_b, Swizzle swizzle_a)
		{
			return new ImageviewPrivate(image, type, base_level, level_count, base_layer, layer_count, swizzle_r, swizzle_g, swizzle_b, swizzle_a);
		}

		SamplerPrivate::SamplerPrivate(Device* _d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			d = (DevicePrivate*)_d;
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

			chk_res(vkCreateSampler(d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)

#endif
		}

		SamplerPrivate::~SamplerPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroySampler(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void SamplerPrivate::release() { delete this; }

		Sampler* Sampler::create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			return new SamplerPrivate(d, mag_filter, min_filter, unnormalized_coordinates);
		}

		uint AtlasTilePrivate::get_index() const { return index; }
		const wchar_t* AtlasTilePrivate::get_filename() const { return filename.c_str(); }
		uint AtlasTilePrivate::get_id() const { return id; }
		Vec2i AtlasTilePrivate::get_pos() const { return pos; }
		Vec2i AtlasTilePrivate::get_size() const { return size; }
		Vec2f AtlasTilePrivate::get_uv0() const { return uv0; }
		Vec2f AtlasTilePrivate::get_uv1() const { return uv1; }

		ImageAtlasPrivate::ImageAtlasPrivate(Device* d, const std::wstring& filename) :
			border(false),
			slot(-1)
		{
			id = FLAME_HASH(filename.c_str());

			std::wstring image_filename;
			auto ini = parse_ini_file(filename);
			for (auto& e : ini.get_section_entries(""))
			{
				if (e.key == "image")
					image_filename = std::filesystem::path(filename).parent_path() / e.value;
				else if (e.key == "border")
					border = !(e.value == "0");
			}

			image = (ImagePrivate*)Image::create(d, image_filename.c_str());

			auto w = (float)image->size.x();
			auto h = (float)image->size.y();

			for (auto& e : ini.get_section_entries("tiles"))
			{
				auto tile = new AtlasTilePrivate;

				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile->filename = s2w(t);
				tile->id = FLAME_HASH(t.c_str());
				ss >> t;
				auto v = stou4(t.c_str());
				tile->pos = Vec2i(v.x(), v.y());
				tile->size = Vec2i(v.z(), v.w());
				tile->uv0.x() = tile->pos.x() / w;
				tile->uv0.y() = tile->pos.y() / h;
				tile->uv1.x() = (tile->pos.x() + tile->size.x()) / w;
				tile->uv1.y() = (tile->pos.y() + tile->size.y()) / h;

				tiles.emplace_back(tile);
			}
		}

		ImageAtlasPrivate::~ImageAtlasPrivate()
		{
			delete image;
		}

		void ImageAtlasPrivate::release() { delete this; }

		bool ImageAtlasPrivate::get_border() const { return border; }

		uint ImageAtlasPrivate::get_tiles_count() const { return tiles.size(); }
		ImageAtlas::Tile* ImageAtlasPrivate::get_tile(uint idx) const { return tiles[idx].get(); }
		ImageAtlas::Tile* ImageAtlasPrivate::find_tile(uint id) const
		{
			for (auto& t : tiles)
			{
				if (t->id == id)
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

			auto atlas = new ImageAtlasPrivate(d, filename);
			report_used_file(filename);

			return atlas;
		}
	}
}

