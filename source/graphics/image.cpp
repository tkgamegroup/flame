#include <flame/foundation/bitmap.h>
#include <flame/foundation/serialize.h>
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
		Format$ Image::find_format(uint channel, uint bpp)
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

		ImagePrivate::ImagePrivate(Device* _d, Format$ _format, const Vec2u& _size, uint _level, uint _layer, SampleCount$ _sample_count, ImageUsage$ usage)
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
			imageInfo.format = to_enum(format);
			imageInfo.extent.width = size.x();
			imageInfo.extent.height = size.y();
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = level;
			imageInfo.arrayLayers = layer;
			imageInfo.samples = to_enum(sample_count);
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = to_flags(usage, format, sample_count);
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

		ImagePrivate::ImagePrivate(Device* _d, Format$ _format, const Vec2u& _size, uint _level, uint _layer, void* native) :
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

		void ImagePrivate::init(const Vec4c& col)
		{
			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_image(this, col);
			cb->change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit({ cb }, nullptr, nullptr, nullptr);
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
			d->gq->submit({ cb }, nullptr, nullptr, nullptr);
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
			d->gq->submit({ cb }, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(stag_buf);
		}

		void Image::init(const Vec4c& col)
		{
			((ImagePrivate*)this)->init(col);
		}

		void Image::get_pixels(const Vec2u& offset, const Vec2u& extent, void* dst)
		{
			((ImagePrivate*)this)->get_pixels(offset, extent, dst);
		}

		void Image::set_pixels(const Vec2u& offset, const Vec2u& extent, const void* src)
		{
			((ImagePrivate*)this)->set_pixels(offset, extent, src);
		}

		Image* Image::create(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, SampleCount$ sample_count, ImageUsage$ usage, void* data)
		{
			auto i = new ImagePrivate(d, format, size, level, layer, sample_count, usage);

			if (data)
			{
				auto staging_buffer = Buffer::create(d, i->data_size, BufferUsageTransferSrc, MemProp$(MemPropHost | MemPropHostCoherent));
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
				d->gq->submit({ cb }, nullptr, nullptr, nullptr);
				d->gq->wait_idle();
				Commandbuffer::destroy(cb);
				Buffer::destroy(staging_buffer);
			}

			return i;
		}

		Image* Image::create_from_bitmap(Device* d, Bitmap* bmp, ImageUsage$ extra_usage)
		{
			auto i = create(d, find_format(bmp->channel, bmp->bpp), bmp->size, 1, 1, SampleCount_1, ImageUsage$(ImageUsageSampled | ImageUsageTransferDst | extra_usage));

			auto staging_buffer = Buffer::create(d, bmp->data_size, BufferUsageTransferSrc, MemProp$(MemPropHost | MemPropHostCoherent));
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
			d->gq->submit({ cb }, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
			Buffer::destroy(staging_buffer);

			return i;
		}

		Image* Image::create_from_file(Device* d, const std::wstring& filename, ImageUsage$ extra_usage)
		{
			std::filesystem::path path(filename);
			if (!std::filesystem::exists(path))
				return nullptr;

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

				staging_buffer = Buffer::create(d, bmp->data_size, BufferUsageTransferSrc, MemProp$(MemPropHost | MemPropHostCoherent));
				staging_buffer->map();
				memcpy(staging_buffer->mapped, bmp->data, staging_buffer->size);
				staging_buffer->unmap();

				Bitmap::destroy(bmp);

				buffer_copy_regions.push_back(BufferImageCopy(Vec2u(width, height)));
			}

			auto i = Image::create(d, fmt, Vec2u(width, height), level, layer, SampleCount_1, ImageUsage$(ImageUsageSampled | ImageUsageTransferDst | extra_usage));

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer, i, buffer_copy_regions.size(), buffer_copy_regions.data());
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit({ cb }, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(staging_buffer);

			return i;
		}

		void Image::save_to_png(Image* i, const std::wstring& filename)
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

		Image* Image::create_from_native(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, void* native)
		{
			return new ImagePrivate(d, format, size, level, layer, native);
		}

		void Image::destroy(Image* i)
		{
			delete (ImagePrivate*)i;
		}

		struct Image$
		{
			AttributeE<Format$> format$i;
			AttributeD<Vec2u> size$i;
			AttributeD<uint> level$i;
			AttributeD<uint> layer$i;
			AttributeE<SampleCount$> sample_count$i;
			AttributeE<ImageUsage$> usage$mi;
			AttributeD<bool> init_with_color$i;
			AttributeD<Vec4c> init_color$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Image$()
			{
				format$i.v = Format_R8G8B8A8_UNORM;
				size$i.v = 4;
				level$i.v = 1;
				layer$i.v = 1;
				usage$mi.v = ImageUsageSampled;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (format$i.b.frame > out$o.b.frame || size$i.b.frame > out$o.b.frame || level$i.b.frame > out$o.b.frame || layer$i.b.frame > out$o.b.frame || sample_count$i.b.frame > out$o.b.frame || usage$mi.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Image::destroy((Image*)out$o.v);
					auto d = Device::default_one();
					if (d && format$i.v != Format_Undefined && size$i.v.x() > 0 && size$i.v.y() > 0 && level$i.v > 0 && layer$i.v > 0)
					{
						out$o.v = Image::create(d, format$i.v, size$i.v, level$i.v, layer$i.v, sample_count$i.v, usage$mi.v);
						if (init_with_color$i.v)
							((Image*)out$o.v)->init(init_color$i.v);
					}
					else
					{
						printf("cannot create image\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Image$()
			{
				if (out$o.v)
					Image::destroy((Image*)out$o.v);
			}
		};

		struct ImageInspector$
		{
			AttributeP<void> in$i;

			AttributeE<Format$> format$o;
			AttributeD<Vec2u> size$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (in$i.b.frame > format$o.b.frame || in$i.b.frame > size$o.b.frame)
				{
					auto image = (Image*)in$i.v;
					if (image)
					{
						format$o.v = image->format;
						size$o.v = image->size;
					}
					else
					{
						printf("cannot inspect image\n");

						format$o.v = Format_Undefined;
						size$o.v = Vec2u(0);
					}
					auto frame = looper().frame;
					format$o.b.frame = frame;
					size$o.b.frame = frame;
				}
			}
		};

		ImageviewPrivate::ImageviewPrivate(Image* _image, ImageviewType$ _type, uint _base_level, uint _level_count, uint _base_layer, uint _layer_count, Swizzle$ _swizzle_r, Swizzle$ _swizzle_g, Swizzle$ _swizzle_b, Swizzle$ _swizzle_a) :
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
			info.components.r = to_enum(swizzle_r);
			info.components.g = to_enum(swizzle_g);
			info.components.b = to_enum(swizzle_b);
			info.components.a = to_enum(swizzle_a);
			info.image = image->v;
			info.viewType = to_enum(type);
			info.format = to_enum(image->format);
			info.subresourceRange.aspectMask = to_flags(aspect_from_format(image->format));
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

		Imageview* Imageview::create(Image* image, ImageviewType$ type, uint base_level, uint level_count, uint base_layer, uint layer_count, Swizzle$ swizzle_r, Swizzle$ swizzle_g, Swizzle$ swizzle_b, Swizzle$ swizzle_a)
		{
			return new ImageviewPrivate(image, type, base_level, level_count, base_layer, layer_count, swizzle_r, swizzle_g, swizzle_b, swizzle_a);
		}

		void Imageview::destroy(Imageview* v)
		{
			delete (ImageviewPrivate*)v;
		}

		struct Imageview$
		{
			AttributeP<void> image$i;
			AttributeE<ImageviewType$> type$i;
			AttributeD<uint> base_level$i;
			AttributeD<uint> level_count$i;
			AttributeD<uint> base_layer$i;
			AttributeD<uint> layer_count$i;
			AttributeE<Swizzle$> swizzle_r$i;
			AttributeE<Swizzle$> swizzle_g$i;
			AttributeE<Swizzle$> swizzle_b$i;
			AttributeE<Swizzle$> swizzle_a$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS Imageview$()
			{
				type$i.v = Imageview2D;
				level_count$i.v = 1;
				layer_count$i.v = 1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (image$i.b.frame > out$o.b.frame || type$i.b.frame > out$o.b.frame || base_level$i.b.frame > out$o.b.frame || base_layer$i.b.frame > out$o.b.frame || layer_count$i.b.frame > out$o.b.frame || swizzle_r$i.b.frame > out$o.b.frame || swizzle_g$i.b.frame > out$o.b.frame || swizzle_b$i.b.frame > out$o.b.frame || swizzle_a$i.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Imageview::destroy((Imageview*)out$o.v);
					if (image$i.v && level_count$i.v > 0 && layer_count$i.v > 0)
						out$o.v = Imageview::create((Image*)image$i.v, type$i.v, base_level$i.v, level_count$i.v, base_layer$i.v, layer_count$i.v, swizzle_r$i.v, swizzle_g$i.v, swizzle_b$i.v, swizzle_a$i.v);
					else
					{
						printf("cannot create imageview\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Imageview$()
			{
				if (out$o.v)
					Imageview::destroy((Imageview*)out$o.v);
			}

		};

		struct ImageviewGeneral$
		{
			AttributeP<void> image$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (image$i.b.frame > out$o.b.frame)
				{
					if (out$o.v)
						Imageview::destroy((Imageview*)out$o.v);
					if (image$i.v)
						out$o.v = Imageview::create((Image*)image$i.v);
					else
					{
						printf("cannot create imageview general\n");

						out$o.v = nullptr;
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~ImageviewGeneral$()
			{
				if (out$o.v)
					Imageview::destroy((Imageview*)out$o.v);
			}
		};

		struct ImageviewsGeneral$
		{
			AttributeP<std::vector<void*>> images$i;

			AttributeD<std::vector<void*>> out$o;

			FLAME_GRAPHICS_EXPORTS ImageviewsGeneral$()
			{
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (images$i.b.frame > out$o.b.frame)
				{
					for (auto i = 0; i < out$o.v.size(); i++)
						Imageview::destroy((Imageview*)out$o.v[i]);
					auto images = get_attribute_vec(images$i);
					if (!images.empty())
					{
						out$o.v.resize(images.size());
						for (auto i = 0; i < out$o.v.size(); i++)
							out$o.v[i] = Imageview::create((Image*)images[i]);
					}
					else
					{
						printf("cannot create imageviews general\n");

						out$o.v.clear();
					}
					out$o.b.frame = looper().frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~ImageviewsGeneral$()
			{
				for (auto i = 0; i < out$o.v.size(); i++)
					Imageview::destroy((Imageview*)out$o.v[i]);
			}
		};

		SamplerPrivate::SamplerPrivate(Device* _d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			d = (DevicePrivate*)_d;
#if defined(FLAME_VULKAN)
			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.magFilter = to_enum(mag_filter);
			info.minFilter = to_enum(min_filter);
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

		AtlasPrivate::AtlasPrivate(Device* d, const std::wstring& filename, const std::wstring& atlas_filename)
		{
			id = H(filename.c_str());

			image = Image::create_from_file(d, filename);
			imageview = Imageview::create(image);

			auto w = (float)image->size.x();
			auto h = (float)image->size.y();

			std::ifstream file(atlas_filename);

			std::string line;
			std::getline(file, line);
			border = line == "1";

			while (!file.eof())
			{
				std::string t;
				Region region;

				std::getline(file, line);
				if (line.empty())
					break;
				std::stringstream ss(line);
				ss >> t;
				region.filename = s2w(t);
				region.id = H(t.c_str());
				ss >> t;
				auto v = stou4(t.c_str());
				region.pos = Vec2i(v.x(), v.y());
				region.size = Vec2i(v.z(), v.w());
				region.uv0.x() = region.pos.x() / w;
				region.uv0.y() = region.pos.y() / h;
				region.uv1.x() = (region.pos.x() + region.size.x()) / w;
				region.uv1.y() = (region.pos.y() + region.size.y()) / h;

				regions.push_back(region);
			}
			file.close();
		}

		Imageview* Atlas::imageview() const
		{
			return ((AtlasPrivate*)this)->imageview;
		}

		const std::vector<Atlas::Region>& Atlas::regions() const
		{
			return ((AtlasPrivate*)this)->regions;
		}

		Atlas* Atlas::load(Device* d, const std::wstring& filename)
		{
			auto atlas_filename = filename + L".atlas";
			if (!std::filesystem::exists(filename) || !std::filesystem::exists(atlas_filename))
				return nullptr;

			auto atlas = new AtlasPrivate(d, filename, atlas_filename);

			return atlas;
		}

		void Atlas::destroy(Atlas* a)
		{
			delete (AtlasPrivate*)a;
		}
	}
}

