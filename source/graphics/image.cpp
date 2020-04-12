#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
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
		Format Image::find_format(uint channel, uint bpp)
		{
			switch (channel)
			{
			case 0:
				switch (bpp)
				{
				case 8:
					return Format_R8_UNORM;
				default:
					return Format_Undefined;

				}
				break;
			case 1:
				switch (bpp)
				{
				case 8:
					return Format_R8_UNORM;
				case 16:
					return Format_R16_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			case 4:
				switch (bpp)
				{
				case 32:
					return Format_R8G8B8A8_UNORM;
				default:
					return Format_Undefined;
				}
				break;
			default:
				return Format_Undefined;
			}
		}

		ImagePrivate::ImagePrivate(Device* _d, Format _format, const Vec2u& _size, uint _level, uint _layer, SampleCount _sample_count, ImageUsageFlags usage)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = _sample_count;

			set_props();

			d = (DevicePrivate*)_d;

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
		}

		ImagePrivate::ImagePrivate(Device* _d, Format _format, const Vec2u& _size, uint _level, uint _layer, void* native) :
			d((DevicePrivate*)_d)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = SampleCount_1;

			set_props();

#if defined(FLAME_VULKAN)
			v = (VkImage)native;
			m = 0;
#elif defined(FLAME_D3D12)
			v = (ID3D12Resource*)native;
#endif
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

			if (dv)
				Imageview::destroy(dv);
		}

		void ImagePrivate::set_props()
		{
			switch (format)
			{
			case Format_R8_UNORM:
				channel = 1;
				bpp = 8;
				break;
			case Format_R16_UNORM:
				channel = 1;
				bpp = 16;
				break;
			case Format_R32_SFLOAT:
				channel = 1;
				bpp = 32;
				break;
			case Format_R8G8B8A8_UNORM: case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
				channel = 4;
				bpp = 32;
				break;
			case Format_R16G16B16A16_UNORM: case Format_R16G16B16A16_SFLOAT:
				channel = 4;
				bpp = 64;
				break;
			case Format_R32G32B32A32_SFLOAT:
				channel = 4;
				bpp = 128;
				break;
			case Format_Depth16:
				channel = 1;
				bpp = 16;
				break;
			default:
				channel = 0;
				bpp = 0;
				assert(0);
			}
			pitch = get_pitch(size.x() * bpp / 8);
			data_size = pitch * size.y();
		}

		void ImagePrivate::change_layout(ImageLayout from, ImageLayout to)
		{
			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, from, to);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
		}

		void ImagePrivate::clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color)
		{
			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, current_layout, ImageLayoutTransferDst);
			cb->clear_image(this, color);
			cb->change_image_layout(this, ImageLayoutTransferDst, after_layout);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
		}

		void ImagePrivate::get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = (bpp / 8) * extent.x() * extent.y();

			auto stag_buf = Buffer::create(d, data_size, BufferUsageTransferDst, MemPropHost);

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(this, stag_buf, 1, &BufferImageCopy(Vec2u(extent), 0, 0, offset));
			cb->change_image_layout(this, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			stag_buf->map();
			memcpy(dst, stag_buf->mapped, stag_buf->size);
			stag_buf->flush();

			Buffer::destroy(stag_buf);
		}

		void ImagePrivate::set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			assert(format == Format_R8_UNORM || format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);

			auto data_size = (bpp / 8) * extent.x() * extent.y();

			auto stag_buf = Buffer::create(d, data_size, BufferUsageTransferSrc, MemPropHost);
			stag_buf->map();
			memcpy(stag_buf->mapped, src, stag_buf->size);
			stag_buf->flush();

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(stag_buf, this, 1, &BufferImageCopy(Vec2u(extent), 0, 0, offset));
			cb->change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(stag_buf);
		}

		Imageview* Image::default_view() const
		{
			return ((ImagePrivate*)this)->dv;
		}

		void Image::change_layout(ImageLayout from, ImageLayout to)
		{
			((ImagePrivate*)this)->change_layout(from, to);
		}

		void Image::clear(ImageLayout current_layout, ImageLayout after_layout, const Vec4c& color)
		{
			((ImagePrivate*)this)->clear(current_layout, after_layout, color);
		}

		void Image::get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			((ImagePrivate*)this)->get_pixels(offset, extent, dst);
		}

		void Image::set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			((ImagePrivate*)this)->set_pixels(offset, extent, src);
		}

		Image* Image::create(Device* d, Format format, const Vec2u& size, uint level, uint layer, SampleCount sample_count, ImageUsageFlags usage, void* data, bool default_view)
		{
			auto i = new ImagePrivate(d, format, size, level, layer, sample_count, usage);
			i->dv = default_view ? (ImageviewPrivate*)Imageview::create(i) : nullptr;

			if (data)
			{
				auto staging_buffer = Buffer::create(d, i->data_size, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, data, staging_buffer->size);
				staging_buffer->unmap();

				auto cb = Commandbuffer::create(d->gcp);
				cb->begin(true);
				cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
				BufferImageCopy copy(i->size);
				cb->copy_buffer_to_image(staging_buffer, i, 1, &copy);
				cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				cb->end();
				d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
				d->gq->wait_idle();
				Commandbuffer::destroy(cb);
				Buffer::destroy(staging_buffer);
			}

			return i;
		}

		Image* Image::create_from_bitmap(Device* d, Bitmap* bmp, ImageUsageFlags extra_usage, bool default_view)
		{
			auto i = (ImagePrivate*)create(d, find_format(bmp->channel, bmp->bpp), bmp->size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);
			i->dv = default_view ? (ImageviewPrivate*)Imageview::create(i) : nullptr;

			auto staging_buffer = Buffer::create(d, bmp->data_size, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
			staging_buffer->map();
			memcpy(staging_buffer->mapped, bmp->data, staging_buffer->size);
			staging_buffer->unmap();

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			BufferImageCopy copy(bmp->size);
			cb->copy_buffer_to_image(staging_buffer, i, 1, &copy);
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
			Buffer::destroy(staging_buffer);

			return i;
		}

		Image* Image::create_from_file(Device* d, const wchar_t* filename, ImageUsageFlags extra_usage, bool default_view)
		{
			std::filesystem::path path(filename);
			if (!std::filesystem::exists(path))
			{
				wprintf(L"cannot find image: %s\n", filename);
				return nullptr;
			}

			int width, height, level, layer;
			auto fmt = Format_Undefined;

			Buffer* staging_buffer;
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

				//auto offset = 0;
				//for (auto i = 0; i < level; i++)
				//{
				//	BufferTextureCopy c;
				//	c.buffer_offset = offset;
				//	c.image_x = 0;
				//	c.image_y = 0;
				//	c.image_width = gli_texture.extent(i).x();
				//	c.image_height = gli_texture.extent(i).y();
				//	c.image_level = i;
				//	buffer_copy_regions.push_back(c);
				//	offset += gli_texture.size(i);
				//}
			}
			else
			{
				auto bmp = Bitmap::create_from_file(filename);
				if (bmp->channel == 3)
					bmp->add_alpha_channel();

				width = bmp->size.x();
				height = bmp->size.y();
				level = layer = 1;

				fmt = find_format(bmp->channel, bmp->bpp);

				staging_buffer = Buffer::create(d, bmp->data_size, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, bmp->data, staging_buffer->size);
				staging_buffer->unmap();

				Bitmap::destroy(bmp);

				buffer_copy_regions.push_back(BufferImageCopy(Vec2u(width, height)));
			}

			auto i = Image::create(d, fmt, Vec2u(width, height), level, layer, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage);

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer, i, buffer_copy_regions.size(), buffer_copy_regions.data());
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(staging_buffer);

			return i;
		}

		void Image::save_to_png(Image* i, const wchar_t* filename)
		{
			if (i->bpp / i->channel <= 8)
			{
				auto bmp = Bitmap::create(i->size, i->channel, i->bpp);
				i->get_pixels(Vec2u(0), i->size, bmp->data);
				Bitmap::save_to_file(bmp, filename);
			}
			else
				printf("cannot save png that has more than 8bit per channel\n");
		}

		Image* Image::create_from_native(Device* d, Format format, const Vec2u& size, uint level, uint layer, void* native, bool default_view)
		{
			auto i = new ImagePrivate(d, format, size, level, layer, native);
			i->dv = default_view ? (ImageviewPrivate*)Imageview::create(i) : nullptr;
			return i;
		}

		void Image::destroy(Image* i)
		{
			delete (ImagePrivate*)i;
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

		Image* Imageview::image() const
		{
			return ((ImageviewPrivate*)this)->image;
		}

		Imageview* Imageview::create(Image* image, ImageviewType type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle swizzle_r, Swizzle swizzle_g, Swizzle swizzle_b, Swizzle swizzle_a)
		{
			return new ImageviewPrivate(image, type, base_level, level_count, base_layer, layer_count, swizzle_r, swizzle_g, swizzle_b, swizzle_a);
		}

		void Imageview::destroy(Imageview* v)
		{
			delete (ImageviewPrivate*)v;
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
			info.mipLodBias = 0.0f;
			info.minLod = 0.0f;
			info.maxLod = 0.0f;

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

		Sampler* Sampler::create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			return new SamplerPrivate(d, mag_filter, min_filter, unnormalized_coordinates);
		}

		void Sampler::destroy(Sampler* s)
		{
			delete (SamplerPrivate*)s;
		}

		AtlasPrivate::~AtlasPrivate()
		{
			Imageview::destroy(imageview);
			Image::destroy(image);
		}

		AtlasPrivate::AtlasPrivate(Device* d, const std::wstring& filename)
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

			image = Image::create_from_file(d, image_filename.c_str());
			imageview = Imageview::create(image);

			auto w = (float)image->size.x();
			auto h = (float)image->size.y();

			for (auto& e : ini.get_section_entries("tiles"))
			{
				auto tile = new AtlasTilePrivate;

				std::string t;
				std::stringstream ss(e.value);
				ss >> t;
				tile->_filename = s2w(t);
				tile->filename = tile->_filename.c_str();
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

		Imageview* Atlas::imageview() const
		{
			return ((AtlasPrivate*)this)->imageview;
		}

		uint Atlas::tile_count() const
		{
			return ((AtlasPrivate*)this)->tiles.size();
		}

		const Atlas::Tile& Atlas::tile(uint idx) const
		{
			return *((AtlasPrivate*)this)->tiles[idx];
		}

		Atlas* Atlas::load(Device* d, const wchar_t* filename)
		{
			if (!std::filesystem::exists(filename))
			{
				wprintf(L"cannot find atlas: %s\n", filename);
				return nullptr;
			}

			auto atlas = new AtlasPrivate(d, filename);
			report_used_file(filename);

			return atlas;
		}

		void Atlas::destroy(Atlas* a)
		{
			delete (AtlasPrivate*)a;
		}
	}
}

