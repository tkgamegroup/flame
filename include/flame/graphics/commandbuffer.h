#pragma once

#include <flame/foundation/foundation.h>
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

			BufferCopy(uint _src_off, uint _dst_off, uint _size) :
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
			FLAME_GRAPHICS_EXPORTS void bind_descriptorset(Descriptorset *s, uint idx);
			FLAME_GRAPHICS_EXPORTS void bind_vertexbuffer(Buffer *b, uint id);
			FLAME_GRAPHICS_EXPORTS void bind_indexbuffer(Buffer *b, IndiceType t);
			FLAME_GRAPHICS_EXPORTS void push_constant(Pipelinelayout* pll, uint offset, uint size, const void *data);
			FLAME_GRAPHICS_EXPORTS void draw(uint count, uint instance_count, uint first_vertex, uint first_instance);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance);
			FLAME_GRAPHICS_EXPORTS void dispatch(const Vec3u& v);

			FLAME_GRAPHICS_EXPORTS void copy_buffer(Buffer *src, Buffer *dst, uint copy_count, BufferCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_image(Image *src, Image *dst, uint copy_count, ImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_buffer_to_image(Buffer *src, Image *dst, uint copy_count, BufferImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void copy_image_to_buffer(Image *src, Buffer *dst, uint copy_count, BufferImageCopy *copies);
			FLAME_GRAPHICS_EXPORTS void change_image_layout(Image *t, ImageLayout from, ImageLayout to, uint base_level = 0, uint level_count = 0, uint base_layer = 0, uint layer_count = 0);

			FLAME_GRAPHICS_EXPORTS void clear_image(Image *i, const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS void end();

			FLAME_GRAPHICS_EXPORTS  static Commandbuffer* create(Commandpool *p, bool sub = false);
			FLAME_GRAPHICS_EXPORTS  static void destroy(Commandbuffer *c);
		};

		struct Queue
		{
			FLAME_GRAPHICS_EXPORTS void wait_idle();
			FLAME_GRAPHICS_EXPORTS void submit(const std::vector<Commandbuffer*> cbs, Semaphore *wait_semaphore, Semaphore *signal_semaphore, Fence* signal_fence);
			FLAME_GRAPHICS_EXPORTS void present(Swapchain *s, Semaphore *wait_semaphore);

			FLAME_GRAPHICS_EXPORTS static Queue *create(Device *d, uint queue_family_idx);
			FLAME_GRAPHICS_EXPORTS static void destroy(Queue *q);
		};
	}
}
