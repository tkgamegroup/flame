// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/foundation.h>
#include <flame/foundation/bitmap.h>
#include "device_private.h"
#include "buffer_private.h"
#include "renderpass_private.h"
#include "pipeline_private.h"
#include "descriptor_private.h"
#include "commandbuffer_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		Format$ Image::find_format(int channel, int bpp)
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

		ImagePrivate::ImagePrivate(Device *_d, Format$ _format, const Vec2u &_size, int _level, int _layer, SampleCount$ _sample_count, int _usage, int _mem_prop)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = _sample_count;

			set_props();

			usage = _usage;
			mem_prop = _mem_prop;
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			VkImageCreateInfo imageInfo;
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.flags = 0;
			imageInfo.pNext = nullptr;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.format = Z(format);
			imageInfo.extent.width = size.x();
			imageInfo.extent.height = size.y();
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = level;
			imageInfo.arrayLayers = layer;
			imageInfo.samples = Z(sample_count);
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.usage = Z((ImageUsage)usage, format, sample_count);
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.queueFamilyIndexCount = 0;
			imageInfo.pQueueFamilyIndices = nullptr;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

			vk_chk_res(vkCreateImage(d->v, &imageInfo, nullptr, &v));

			VkMemoryRequirements memRequirements;
			vkGetImageMemoryRequirements(d->v, v, &memRequirements);

			VkMemoryAllocateInfo allocInfo;
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = d->find_memory_type(memRequirements.memoryTypeBits, Z((MemProp)mem_prop));

			vk_chk_res(vkAllocateMemory(d->v, &allocInfo, nullptr, &m));

			vk_chk_res(vkBindImageMemory(d->v, v, m, 0));
#elif defined(FLAME_D3D12)

#endif
		}

		ImagePrivate::ImagePrivate(Device *_d, Format$ _format, const Vec2u &_size, int _level, int _layer, void *native)
		{
			format = _format;
			size = _size;
			level = _level;
			layer = _layer;
			sample_count = SampleCount_1;

			set_props();

			usage = 0;
			mem_prop = 0;
			d = (DevicePrivate*)_d;

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
				channel_ = 1;
				bpp_ = 8;
				break;
			case Format_R16_UNORM:
				channel_ = 1;
				bpp_ = 16;
				break;
			case Format_R32_SFLOAT:
				channel_ = 1;
				bpp_ = 32;
				break;
			case Format_R8G8B8A8_UNORM: case Format_B8G8R8A8_UNORM: case Format_Swapchain_B8G8R8A8_UNORM:
				channel_ = 4;
				bpp_ = 32;
				break;
			case Format_R16G16B16A16_UNORM: case Format_R16G16B16A16_SFLOAT:
				channel_ = 4;
				bpp_ = 64;
				break;
			case Format_R32G32B32A32_SFLOAT:
				channel_ = 4;
				bpp_ = 128;
				break;
			case Format_Depth16:
				channel_ = 1;
				bpp_ = 16;
				break;
			default:
				channel_ = 0;
				bpp_ = 0;
				assert(0);
			}
			pitch_ = Bitmap::get_pitch(size.x(), bpp_);
			data_size_ = pitch_ * size.y();
		}

		void ImagePrivate::init(const Vec4c &col)
		{
			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->clear_image(this, col);
			cb->change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
		}

		void ImagePrivate::get_pixels(int x, int y, int cx, int cy, void *dst)
		{
			assert(format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);
			if (cx == -1)
				cx = size.x();
			if (cy == -1)
				cy = size.y();

			auto data_size = (bpp_ / 8) * cx * cy;

			auto stag_buf = Buffer::create(d, data_size, BufferUsageTransferDst, MemPropHost);

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferSrc);
			cb->copy_image_to_buffer(this, stag_buf, 1, &BufferImageCopy(cx, cy, 0, 0, x, y));
			cb->change_image_layout(this, ImageLayoutTransferSrc, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			stag_buf->map();
			memcpy(dst, stag_buf->mapped, stag_buf->size);
			stag_buf->flush();

			Buffer::destroy(stag_buf);
		}

		void ImagePrivate::set_pixels(int x, int y, int cx, int cy, const void *src)
		{
			assert(format == Format_R8G8B8A8_UNORM || format == Format_R16G16B16A16_UNORM);
			if (cx == -1)
				cx = size.x();
			if (cy == -1)
				cy = size.y();

			auto data_size = (bpp_ / 8) * cx * cy;

			auto stag_buf = Buffer::create(d, data_size, BufferUsageTransferSrc, MemPropHost);
			stag_buf->map();
			memcpy(stag_buf->mapped, src, stag_buf->size);
			stag_buf->flush();

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(this, ImageLayoutShaderReadOnly, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(stag_buf, this, 1, &BufferImageCopy(cx, cy, 0, 0, x, y));
			cb->change_image_layout(this, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(stag_buf);
		}

		void ImagePrivate::save_png(const wchar_t *filename)
		{
			if (bpp_ / channel_ > 8)
			{
				auto img = Image::create(d, Format_R8G8B8A8_UNORM, size, 1, 1, SampleCount_1, ImageUsageAttachment | ImageUsageTransferSrc, MemPropDevice);
				auto img_v = Imageview::create(img);

				FramebufferInfo fb_info;
				fb_info.rp = d->rp_one_rgba32;
				fb_info.views.push_back(img_v);
				auto fb = Framebuffer::create(d, fb_info);
				auto ds = Descriptorset::create(d->dp, d->pl_trans->layout()->dsl(0));
				auto cb = Commandbuffer::create(d->gcp);

				auto my_view = Imageview::create(this);
				ds->set_imageview(0, 0, my_view, d->sp_bi_linear);

				cb->begin(true);
				cb->begin_renderpass(d->rp_one_rgba32, fb, nullptr);

				auto vp = Vec4f(Vec2f(0.f), Vec2f(size));
				cb->set_viewport(vp);
				cb->set_scissor(vp);

				cb->bind_pipeline(d->pl_trans);
				cb->bind_descriptorset(ds, 0);
				cb->draw(3, 1, 0, 0);

				cb->end_renderpass();
				cb->end();
				d->gq->submit(cb, nullptr, nullptr, nullptr);
				d->gq->wait_idle();

				Imageview::destroy(my_view);

				Framebuffer::destroy(fb);
				Commandbuffer::destroy(cb);

				img->save_png(filename);

				Imageview::destroy(img_v);
				Image::destroy(img);
			}
			else
			{
				auto bmp = Bitmap::create(size, channel_, bpp_);
				get_pixels(0, 0, -1, -1, bmp->data);
				bmp->save(filename);
			}
		}

		void Image::init(const Vec4c &col)
		{
			((ImagePrivate*)this)->init(col);
		}

		void Image::get_pixels(int x, int y, int cx, int cy, void *dst)
		{
			((ImagePrivate*)this)->get_pixels(x, y, cx, cy, dst);
		}

		void Image::set_pixels(int x, int y, int cx, int cy, const void *src)
		{
			((ImagePrivate*)this)->set_pixels(x, y, cx, cy, src);
		}

		void Image::save_png(const wchar_t *filename)
		{
			((ImagePrivate*)this)->save_png(filename);
		}

		Image *Image::create(Device *d, Format$ format, const Vec2u &size, int level, int layer, SampleCount$ sample_count, int usage, int mem_prop, void *data)
		{
			auto i = new ImagePrivate(d, format, size, level, layer, sample_count, usage, mem_prop);

			if (data)
			{
				auto staging_buffer = Buffer::create(d, i->data_size_, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
				staging_buffer->map();
				memcpy(staging_buffer->mapped, data, staging_buffer->size);
				staging_buffer->unmap();

				auto cb = Commandbuffer::create(d->gcp);
				cb->begin(true);
				cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
				BufferImageCopy copy(i->size.x(), i->size.y());
				cb->copy_buffer_to_image(staging_buffer, i, 1, &copy);
				cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
				cb->end();
				d->gq->submit(cb, nullptr, nullptr, nullptr);
				d->gq->wait_idle();
				Commandbuffer::destroy(cb);
				Buffer::destroy(staging_buffer);
			}

			return i;
		}

		Image *Image::create_from_bitmap(Device *d, Bitmap *bmp, int extra_usage)
		{
			auto i = create(d, find_format(bmp->channel, bmp->bpp), bmp->size, 1, 1, SampleCount_1, ImageUsageSampled | ImageUsageTransferDst | extra_usage, MemPropDevice);

			auto staging_buffer = Buffer::create(d, bmp->data_size, BufferUsageTransferSrc, MemPropHost | MemPropHostCoherent);
			staging_buffer->map();
			memcpy(staging_buffer->mapped, bmp->data, staging_buffer->size);
			staging_buffer->unmap();

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			BufferImageCopy copy(bmp->size.x(), bmp->size.y());
			cb->copy_buffer_to_image(staging_buffer, i, 1, &copy);
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);
			Buffer::destroy(staging_buffer);

			return i;

		}

		Image *Image::create_from_file(Device *d, const wchar_t *filename, int extra_usage)
		{
			std::fs::path path(filename);
			if (!std::fs::exists(path))
				return nullptr;

			int width, height, level, layer;
			auto fmt = Format_Undefined;

			Buffer *staging_buffer;
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

				buffer_copy_regions.push_back(BufferImageCopy(width, height));
			}

			auto i = Image::create(d, fmt, Vec2u(width, height), level, layer, SampleCount_1,
				ImageUsageSampled | ImageUsageTransferDst | extra_usage, MemPropDevice);

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->change_image_layout(i, ImageLayoutUndefined, ImageLayoutTransferDst);
			cb->copy_buffer_to_image(staging_buffer, i, buffer_copy_regions.size(), buffer_copy_regions.data());
			cb->change_image_layout(i, ImageLayoutTransferDst, ImageLayoutShaderReadOnly);
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(staging_buffer);

			return i;
		}

		Image *Image::create_from_native(Device *d, Format$ format, const Vec2u &size, int level, int layer, void *native)
		{
			return new ImagePrivate(d, format, size, level, layer, native);
		}

		void Image::destroy(Image *i)
		{
			delete (ImagePrivate*)i;
		}

		ImageviewPrivate::ImageviewPrivate(Image *_i, ImageviewType _type, int _base_level, int _level_count, int _base_layer, int _layer_count, ComponentMapping *_mapping)
		{
			i = (ImagePrivate*)_i;
			type = _type;
			base_level = _base_level;
			level_count = _level_count;
			base_layer = _base_layer;
			layer_count = _layer_count;

			if (_mapping)
				mapping = *_mapping;
			else
				mapping.r = mapping.g = mapping.b = mapping.a = SwizzleIdentity;

#if defined(FLAME_VULKAN)
			VkImageViewCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.components.r = Z(mapping.r);
			info.components.g = Z(mapping.g);
			info.components.b = Z(mapping.b);
			info.components.a = Z(mapping.a);
			info.image = i->v;
			info.viewType = Z(type);
			info.format = Z(i->format);
			info.subresourceRange.aspectMask = Z(aspect_from_format(i->format));
			info.subresourceRange.baseMipLevel = base_level;
			info.subresourceRange.levelCount = level_count;
			info.subresourceRange.baseArrayLayer = base_layer;
			info.subresourceRange.layerCount = layer_count;

			vk_chk_res(vkCreateImageView(i->d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)
			D3D12_DESCRIPTOR_HEAP_DESC desc = {};
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			desc.NumDescriptors = 1;
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			auto d = i->d->v;
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
			vkDestroyImageView(i->d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Image* Imageview::image() const
		{
			return ((ImageviewPrivate*)this)->i;
		}

		Imageview* Imageview::create(Image *_i, ImageviewType type, int base_level, int level_count, int base_layer, int layer_count, ComponentMapping *mapping)
		{
			return new ImageviewPrivate(_i, type, base_level, level_count, base_layer, layer_count, mapping);
		}

		void Imageview::destroy(Imageview *v)
		{
			delete (ImageviewPrivate*)v;
		}

		SamplerPrivate::SamplerPrivate(Device *_d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			d = (DevicePrivate*)_d;
#if defined(FLAME_VULKAN)
			VkSamplerCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			info.flags = 0;
			info.pNext = nullptr;
			info.magFilter = Z(mag_filter);
			info.minFilter = Z(min_filter);
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

			vk_chk_res(vkCreateSampler(d->v, &info, nullptr, &v));
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

		Sampler *Sampler::create(Device *d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates)
		{
			return new SamplerPrivate(d, mag_filter, min_filter, unnormalized_coordinates);
		}

		void Sampler::destroy(Sampler *s)
		{
			delete (SamplerPrivate*)s;
		}
	}
}

