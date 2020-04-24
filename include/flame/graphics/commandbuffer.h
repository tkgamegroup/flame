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
		struct Framebuffer;
		struct Pipeline;
		struct Pipelinelayout;
		struct Descriptorset;
		struct Swapchain;
		struct Semaphore;
		struct Fence;

		struct Commandpool
		{
			FLAME_GRAPHICS_EXPORTS static Commandpool* get_default(QueueFamily family);
			FLAME_GRAPHICS_EXPORTS static void set_default(Commandpool* graphics, Commandpool* transfer);
			FLAME_GRAPHICS_EXPORTS static Commandpool* create(Device* d, int queue_family_idx);
			FLAME_GRAPHICS_EXPORTS static void destroy(Commandpool* p);
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

			ImageCopy() :
				src_off(0),
				dst_off(0),
				size(0)
			{
			}
		};

		struct BufferImageCopy
		{
			uint buffer_offset;
			Vec2u image_offset;
			Vec2u image_extent;
			uint image_level;

			BufferImageCopy()
			{
			}

			BufferImageCopy(const Vec2u& image_extent, uint buf_off = 0, uint level = 0, const Vec2u& image_offset = Vec2u(0)) :
				buffer_offset(buf_off),
				image_offset(image_offset),
				image_extent(image_extent),
				image_level(level)
			{
			}
		};

		struct Commandbuffer
		{
			FLAME_GRAPHICS_EXPORTS void begin(bool once = false);

			FLAME_GRAPHICS_EXPORTS void begin_renderpass(Framebuffer* fb, uint clearvalue_count, const Vec4f* clearvalues);
			FLAME_GRAPHICS_EXPORTS void end_renderpass();
			FLAME_GRAPHICS_EXPORTS void set_viewport(const Vec4f& rect);
			FLAME_GRAPHICS_EXPORTS void set_scissor(const Vec4f& rect);
			FLAME_GRAPHICS_EXPORTS void bind_pipeline(Pipeline* p);
			FLAME_GRAPHICS_EXPORTS void bind_descriptorset(Descriptorset* s, uint idx, Pipelinelayout* pll = nullptr);
			FLAME_GRAPHICS_EXPORTS void bind_vertexbuffer(Buffer* b, uint id);
			FLAME_GRAPHICS_EXPORTS void bind_indexbuffer(Buffer* b, IndiceType t);
			FLAME_GRAPHICS_EXPORTS void push_constant(uint offset, uint size, const void* data, Pipelinelayout* pll = nullptr);
			FLAME_GRAPHICS_EXPORTS void draw(uint count, uint instance_count, uint first_vertex, uint first_instance);
			FLAME_GRAPHICS_EXPORTS void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance);
			FLAME_GRAPHICS_EXPORTS void dispatch(const Vec3u& v);

			FLAME_GRAPHICS_EXPORTS void copy_buffer(Buffer* src, Buffer* dst, uint copy_count, BufferCopy* copies);
			FLAME_GRAPHICS_EXPORTS void copy_image(Image* src, Image* dst, uint copy_count, ImageCopy* copies);
			FLAME_GRAPHICS_EXPORTS void copy_buffer_to_image(Buffer* src, Image* dst, uint copy_count, BufferImageCopy* copies);
			FLAME_GRAPHICS_EXPORTS void copy_image_to_buffer(Image* src, Buffer* dst, uint copy_count, BufferImageCopy* copies);
			FLAME_GRAPHICS_EXPORTS void change_image_layout(Image* t, ImageLayout from, ImageLayout to, uint base_level = 0, uint level_count = 0, uint base_layer = 0, uint layer_count = 0);

			FLAME_GRAPHICS_EXPORTS void clear_image(Image* i, const Vec4c& col);

			FLAME_GRAPHICS_EXPORTS void end();

			FLAME_GRAPHICS_EXPORTS  static Commandbuffer* create(Commandpool* p, bool sub = false);
			FLAME_GRAPHICS_EXPORTS  static void destroy(Commandbuffer* c);
		};

		struct Queue
		{
			FLAME_GRAPHICS_EXPORTS void wait_idle();
			FLAME_GRAPHICS_EXPORTS void submit(uint cb_count, Commandbuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence);
			FLAME_GRAPHICS_EXPORTS void present(Swapchain* s, Semaphore* wait_semaphore);

			FLAME_GRAPHICS_EXPORTS static Queue* get_default(QueueFamily family);
			FLAME_GRAPHICS_EXPORTS static void set_default(Queue* graphics, Queue* transfer);
			FLAME_GRAPHICS_EXPORTS static Queue* create(Device* d, uint queue_family_idx);
			FLAME_GRAPHICS_EXPORTS static void destroy(Queue* q);
		};
	}
}
