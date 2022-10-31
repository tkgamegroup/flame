#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct CommandPool
		{
			virtual ~CommandPool() {}

			struct Get
			{
				virtual CommandPoolPtr operator()(QueueFamily family = QueueGraphics) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Create
			{
				virtual CommandPoolPtr operator()(int queue_family_idx) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct BufferCopy
		{
			uint src_off = 0;
			uint dst_off = 0;
			uint size;

			BufferCopy() 
			{
			}

			BufferCopy(uint size) : 
				size(size) 
			{
			}

			BufferCopy(uint off, uint size) : 
				src_off(off), dst_off(off), size(size) 
			{
			}

			BufferCopy(uint src_off, uint dst_off, uint size) :
				src_off(src_off), dst_off(dst_off), size(size)
			{
			}
		};

		struct ImageCopy
		{
			uvec3 src_off = uvec3(0);
			ImageSub src_sub;
			uvec3 dst_off = uvec3(0);
			ImageSub dst_sub;
			uvec3 ext;
		};

		struct BufferImageCopy
		{
			uint buf_off = 0;
			uvec3 img_off = uvec3(0);
			uvec3 img_ext;
			ImageSub img_sub;

			BufferImageCopy()
			{
			}

			BufferImageCopy(const uvec3& ext) :
				img_ext(ext)
			{
			}
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
			uint instance_count = 1;
			uint first_vertex = 0;
			uint first_instance = 0;
		};

		struct DrawIndexedIndirectCommand
		{
			uint index_count;
			uint instance_count = 1;
			uint first_index = 0;
			int  vertex_offset = 0;
			uint first_instance = 0;
		};

		struct CommandBuffer
		{
			uint64 last_executed_time = 0;
			bool want_executed_time = false;

			virtual ~CommandBuffer() {}

			virtual void begin(bool once = false) = 0;

			virtual void begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs = nullptr) = 0;
			inline void begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, std::initializer_list<vec4> cvs)
			{
				begin_renderpass(rp, fb, (vec4*)cvs.begin());
			}
			virtual void next_pass() = 0;
			virtual void end_renderpass() = 0;
			virtual void set_viewport(const Rect& rect) = 0;
			virtual void set_scissor(const Rect& rect) = 0;
			inline void set_viewport_and_scissor(const Rect& rect) { set_viewport(rect); set_scissor(rect); }
			virtual void bind_pipeline_layout(PipelineLayoutPtr pll, PipelineType plt = PipelineGraphics) = 0;
			virtual void bind_pipeline(GraphicsPipelinePtr pl) = 0;
			virtual void bind_pipeline(ComputePipelinePtr pl) = 0;
			virtual void bind_descriptor_sets(uint idx, std::span<DescriptorSetPtr> descriptor_sets) = 0;
			inline void bind_descriptor_set(uint idx, DescriptorSetPtr descriptor_set)
			{
				bind_descriptor_sets(idx, { &descriptor_set, 1 });
			}
			virtual void bind_vertex_buffer(BufferPtr buf, uint id) = 0;
			virtual void bind_index_buffer(BufferPtr buf, IndiceType t) = 0;
			virtual void push_constant(uint offset, uint size, const void* data) = 0;
			template<typename T> inline void push_constant_t(const T& data, uint offset = 0)
			{
				push_constant(offset, sizeof(T), &data);
			}
			virtual void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) = 0;
			virtual void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) = 0;
			virtual void draw_indirect(BufferPtr buf, uint offset, uint count) = 0;
			virtual void draw_indexed_indirect(BufferPtr buf, uint offset, uint count) = 0;
			virtual void dispatch(const uvec3& v) = 0;
			virtual void buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, 
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) = 0;
			virtual void image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout new_layout, 
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone,
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) = 0;

			virtual void copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies) = 0;
			inline void copy_buffer(BufferPtr src, BufferPtr dst, const BufferCopy& cpy)
			{
				copy_buffer(src, dst, { (BufferCopy*)&cpy, 1});
			}
			virtual void copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies) = 0;
			inline void copy_image(ImagePtr src, ImagePtr dst, const ImageCopy& cpy)
			{
				copy_image(src, dst, { (ImageCopy*)&cpy, 1 });
			}
			virtual void copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies) = 0;
			inline void copy_buffer_to_image(BufferPtr src, ImagePtr dst, const BufferImageCopy& cpy)
			{
				copy_buffer_to_image(src, dst, { (BufferImageCopy*)&cpy, 1 });
			}
			virtual void copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies) = 0;
			inline void copy_image_to_buffer(ImagePtr src, BufferPtr dst, const BufferImageCopy& cpy)
			{
				copy_image_to_buffer(src, dst, { (BufferImageCopy*)&cpy, 1 });
			}
			virtual void blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter) = 0;

			virtual void clear_color_image(ImagePtr img, const ImageSub& sub, const vec4& color) = 0;
			virtual void clear_depth_image(ImagePtr img, const ImageSub& sub, float depth) = 0;

			virtual void end() = 0;

			virtual void calc_executed_time() = 0;

			struct Create
			{
				virtual CommandBufferPtr operator()(CommandPoolPtr p, bool sub = false) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct Queue
		{
			virtual ~Queue() {}

			virtual void wait_idle() = 0;
			virtual void submit(std::span<CommandBufferPtr> commandbuffers, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence) = 0;
			inline void submit1(CommandBufferPtr commandbuffer, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence)
			{
				CommandBufferPtr cbs[] = { commandbuffer };
				submit({ cbs, 1 }, wait_semaphore, signal_semaphore, signal_fence);
			}
			virtual void present(SwapchainPtr swapchain, SemaphorePtr wait_semaphore) = 0;

			struct Get
			{
				virtual QueuePtr operator()(QueueFamily family = QueueGraphics) = 0;
			};
			FLAME_GRAPHICS_API static Get& get;

			struct Create
			{
				virtual QueuePtr operator()(uint queue_family_idx) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct Semaphore
		{
			virtual ~Semaphore() {}

			struct Create
			{
				virtual SemaphorePtr operator()() = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct Fence
		{
			virtual ~Fence() {}

			virtual void wait(bool auto_reset = true) = 0;

			struct Create
			{
				virtual FencePtr operator()(bool signaled = true) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
