// MIT License
// 
// Copyright (c) 2018 wjs
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

#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Image;
		struct Renderpass;
		struct Clearvalues;
		struct Framebuffer;
		struct Pipeline;
		struct Pipelinelayout;
		struct Descriptorset;
		struct Swapchain;
		struct Semaphore;
		struct Fence;

		struct Commandpool
		{
			FLAME_GRAPHICS_EXPORTS static Commandpool *create(Device *d, int queue_family_idx);
			FLAME_GRAPHICS_EXPORTS static void destroy(Commandpool *p);
		};

		struct BufferCopy
		{
			uint src_off;
			uint dst_off;
			uint size;

			BufferCopy() 
			{
			}

			BufferCopy(int _src_off, int _dst_off, int _size) :
				src_off(_src_off),
				dst_off(_dst_off),
				size(_size)
			{
			}
		};

		struct ImageCopy
		{
			Vec2u src_off;
			Vec2u dst_off;
			Vec2u size;

			ImageCopy()
			{
			}

			ImageCopy(const Vec2u &_src_off, const Vec2u &_dst_off, const Vec2u &_size) :
				src_off(_src_off),
				dst_off(_dst_off),
				size(_size)
			{
			}
		};

		struct BufferImageCopy
		{
			uint buffer_offset;
			uint image_x;
			uint image_y;
			uint image_width;
			uint image_height;
			uint image_level;

			BufferImageCopy()
			{
			}

			BufferImageCopy(uint w, uint h, uint buf_off = 0, uint level = 0, uint x = 0, uint y = 0) :
				buffer_offset(buf_off),
				image_x(x),
				image_y(y),
				image_width(w),
				image_height(h),
				image_level(level)
			{
			}
		};

		struct Commandbuffer
		{
			FLAME_GRAPHICS_EXPORTS void begin(bool once = false);

			FLAME_GRAPHICS_EXPORTS void begin_renderpass(Renderpass *r, Framebuffer *f, Clearvalues *cv);
			FLAME_GRAPHICS_EXPORTS void end_renderpass();
			FLAME_GRAPHICS_EXPORTS void set_viewport(const Vec4f& rect);
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Vec4f& rect);
			FLAME_GRAPHICS_EXPORTS void bind_pipeline(Pipeline *p);
			FLAME_GRAPHICS_EXPORTS void bind_descriptorset(Descriptorset *s, int idx);
			FLAME_GRAPHICS_EXPORTS void bind_vertexbuffer(Buffer *b, int id);
			FLAME_GRAPHICS_EXPORTS void bind_indexbuffer(Buffer *b, IndiceType t);
			FLAME_GRAPHICS_EXPORTS void push_constant(int offset, int size, const void *data, Pipelinelayout *layout = nullptr);
			FLAME_GRAPHICS_EXPORTS void draw(int count, int instance_count, int first_vertex, int first_instance);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(int count, int first_index, int vertex_offset, int instance_count, int first_instance);
			FLAME_GRAPHICS_EXPORTS void dispatch(const Vec3u& v);

			FLAME_GRAPHICS_EXPORTS void copy_buffer(Buffer *src, Buffer *dst, int copy_count, BufferCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_image(Image *src, Image *dst, int copy_count, ImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_buffer_to_image(Buffer *src, Image *dst, int copy_count, BufferImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_image_to_buffer(Image *src, Buffer *dst, int copy_count, BufferImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void change_image_layout(Image *t, ImageLayout from, ImageLayout to,
				int base_level = 0, int level_count = 0, int base_layer = 0, int layer_count = 0);

			FLAME_GRAPHICS_EXPORTS void clear_image(Image *i, const Vec4b &col);

			FLAME_GRAPHICS_EXPORTS void end();

			FLAME_GRAPHICS_EXPORTS  static Commandbuffer* create(Commandpool *p, bool sub = false);
			FLAME_GRAPHICS_EXPORTS  static void destroy(Commandbuffer *c);
		};

		struct Queue
		{
			FLAME_GRAPHICS_EXPORTS void wait_idle();
			FLAME_GRAPHICS_EXPORTS void submit(Commandbuffer *c, Semaphore *wait_semaphore, Semaphore *signal_semaphore, Fence* signal_fence);
			FLAME_GRAPHICS_EXPORTS void present(Swapchain *s, Semaphore *wait_semaphore);

			FLAME_GRAPHICS_EXPORTS static Queue *create(Device *d, int queue_family_idx);
			FLAME_GRAPHICS_EXPORTS static void destroy(Queue *q);
		};
	}
}
