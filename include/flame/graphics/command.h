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
			uint src_off = 0;
			uint dst_off = 0;
			uint size;
		};

		struct ImageCopy
		{
			Vec2u src_off = Vec2u(0);
			Vec2u dst_off = Vec2u(0);
			Vec2u size;
		};

		struct BufferImageCopy
		{
			uint buffer_offset = 0;
			Vec2u image_offset = Vec2u(0);
			Vec2u image_extent;
			uint image_level = 0;
		};

		struct DrawIndirectCommand
		{
			uint vertex_count;
			uint instance_count;
			uint first_vertex;
			uint first_instance;
		};

		struct DrawIndexedIndirectCommand
		{
			uint index_count;
			uint instance_count = 1;
			uint first_index;
			int  vertex_offset;
			uint first_instance;
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
			template <class T>
			inline void push_constant_t(uint offset, const T& data, PipelineLayout* pll = nullptr)
			{
				push_constant(offset, sizeof(T), &data, pll);
			}
			virtual void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) = 0;
			virtual void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) = 0;
			virtual void draw_indirect(Buffer* b, uint offset, uint count) = 0;
			virtual void draw_indexed_indirect(Buffer* b, uint offset, uint count) = 0;
			virtual void dispatch(const Vec3u& v) = 0;
			virtual void buffer_barrier(Buffer* b, AccessFlags src_access, AccessFlags dst_access) = 0;
			virtual void image_barrier(Image* i, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, 
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone) = 0;

			virtual void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) = 0;
			virtual void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) = 0;
			virtual void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) = 0;

			virtual void clear_color_image(Image* i, const Vec4c& color) = 0;
			virtual void clear_depth_image(Image* i, float depth) = 0;

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
