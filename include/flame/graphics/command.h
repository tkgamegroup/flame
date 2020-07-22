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
		struct PipelineLayout;
		struct DescriptorSet;
		struct Swapchain;
		struct Semaphore;
		struct Fence;

		struct CommandPool
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static CommandPool* create(Device* d, int queue_family_idx);
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

		struct CommandBuffer
		{
			virtual void release() = 0;

			virtual void begin(bool once = false) = 0;

			virtual void begin_renderpass(Framebuffer* fb, const Vec4f* clearvalues) = 0;
			virtual void end_renderpass() = 0;
			virtual void set_viewport(const Vec4f& rect) = 0;
			virtual void set_scissor(const Vec4f& rect) = 0;
			virtual void bind_pipeline(Pipeline* p) = 0;
			virtual void bind_descriptor_set(DescriptorSet* s, uint idx, PipelineLayout* pll = nullptr) = 0;
			virtual void bind_vertex_buffer(Buffer* b, uint id) = 0;
			virtual void bind_index_buffer(Buffer* b, IndiceType t) = 0;
			virtual void push_constant(uint offset, uint size, const void* data, PipelineLayout* pll = nullptr) = 0;
			virtual void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) = 0;
			virtual void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) = 0;
			virtual void dispatch(const Vec3u& v) = 0;

			virtual void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) = 0;
			virtual void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) = 0;
			virtual void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void change_image_layout(Image* t, ImageLayout from, ImageLayout to, uint base_level = 0, uint level_count = 0, uint base_layer = 0, uint layer_count = 0) = 0;

			virtual void clear_image(Image* i, const Vec4c& col) = 0;

			virtual void end() = 0;

			FLAME_GRAPHICS_EXPORTS  static CommandBuffer* create(CommandPool* p, bool sub = false);
		};

		struct Queue
		{
			virtual void release() = 0;

			virtual void wait_idle() = 0;
			virtual void submit(uint cbs_count, CommandBuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence) = 0;
			virtual void present(Swapchain* s, Semaphore* wait_semaphore) = 0;

			FLAME_GRAPHICS_EXPORTS static Queue* create(Device* d, uint queue_family_idx);
		};
	}
}
