#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct CommandPool
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static CommandPool* get(Device* device, QueueFamily family = QueueGraphics);
			FLAME_GRAPHICS_EXPORTS static CommandPool* create(Device* device, int queue_family_idx);
		};

		struct BufferCopy
		{
			uint src_off = 0;
			uint dst_off = 0;
			uint size;
		};

		struct ImageCopy
		{
			uvec2 src_off = uvec2(0);
			ImageSub src_sub;
			uvec2 dst_off = uvec2(0);
			ImageSub dst_sub;
			uvec2 size;
		};

		struct BufferImageCopy
		{
			uint buf_off = 0;
			uvec2 img_off = uvec2(0);
			uvec2 img_ext;
			ImageSub img_sub;
		};

		struct ImageBlit
		{
			ImageSub src_sub;
			ivec4 src_range;
			ImageSub dst_sub;
			ivec4 dst_range;
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

			virtual void begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs = nullptr) = 0;
			virtual void next_pass() = 0;
			virtual void end_renderpass() = 0;
			virtual void set_viewport(const Rect& rect) = 0;
			virtual void set_scissor(const Rect& rect) = 0;
			virtual void bind_pipeline_layout(PipelineLayoutPtr pll, PipelineType plt = PipelineGraphics) = 0;
			virtual void bind_pipeline(PipelinePtr pl) = 0;
			virtual void bind_descriptor_sets(uint idx, uint cnt, DescriptorSetPtr* dss) = 0;
			inline void bind_descriptor_set(uint idx, DescriptorSetPtr ds)
			{
				DescriptorSetPtr dss[] = { ds };
				bind_descriptor_sets(idx, 1, dss);
			}
			virtual void bind_vertex_buffer(BufferPtr buf, uint id) = 0;
			virtual void bind_index_buffer(BufferPtr buf, IndiceType t) = 0;
			virtual void push_constant(uint offset, uint size, const void* data) = 0;
			template <class T>
			inline void push_constant_t(const T& data)
			{
				push_constant(0, sizeof(T), &data);
			}
			virtual void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) = 0;
			virtual void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) = 0;
			virtual void draw_indirect(BufferPtr buf, uint offset, uint count) = 0;
			virtual void draw_indexed_indirect(BufferPtr buf, uint offset, uint count) = 0;
			virtual void dispatch(const uvec3& v) = 0;
			virtual void buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, 
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) = 0;
			virtual void image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout old_layout, ImageLayout new_layout, 
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone,
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) = 0;

			virtual void copy_buffer(BufferPtr src, BufferPtr dst, uint copies_count, BufferCopy* copies) = 0;
			virtual void copy_image(ImagePtr src, ImagePtr dst, uint copies_count, ImageCopy* copies) = 0;
			virtual void copy_buffer_to_image(BufferPtr src, ImagePtr dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void copy_image_to_buffer(ImagePtr src, BufferPtr dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void blit_image(ImagePtr src, ImagePtr dst, uint blits_count, ImageBlit* blits, Filter filter) = 0;

			virtual void clear_color_image(ImagePtr img, const ImageSub& sub, const cvec4& color) = 0;
			virtual void clear_depth_image(ImagePtr img, const ImageSub& sub, float depth) = 0;

			virtual void end() = 0;

			FLAME_GRAPHICS_EXPORTS  static CommandBuffer* create(CommandPool* p, bool sub = false);
		};

		struct Queue
		{
			virtual void release() = 0;

			virtual void wait_idle() = 0;
			virtual void submit(uint cbs_count, CommandBufferPtr* cbs, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence) = 0;
			virtual void present(SwapchainPtr swapchain, SemaphorePtr wait_semaphore) = 0;

			FLAME_GRAPHICS_EXPORTS static Queue* get(Device* device, QueueFamily family = QueueGraphics);
			FLAME_GRAPHICS_EXPORTS static Queue* create(Device* device, uint queue_family_idx);
		};

		struct Event
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Event* create(Device* device);
		};

		struct Semaphore
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Semaphore* create(Device* device);
		};

		struct Fence
		{
			virtual void release() = 0;

			virtual void wait(bool auto_reset = true) = 0;

			FLAME_GRAPHICS_EXPORTS static Fence* create(Device* device, bool signaled = true);
		};

		struct InstanceCB : UniPtr<CommandBuffer>
		{
			Device* device;
			Fence* fence;

			InstanceCB(Device* device, Fence* fence = nullptr) :
				device(device),
				fence(fence)
			{
				reset(CommandBuffer::create(CommandPool::get(device)));
				p->begin(true);
			}

			~InstanceCB()
			{
				p->end();
				auto q = Queue::get(device);
				q->submit(1, (CommandBufferPtr*)&p, nullptr, nullptr, (FencePtr)fence);
				if (!fence)
					q->wait_idle();
				else
					fence->wait();
			}
		};
	}
}
