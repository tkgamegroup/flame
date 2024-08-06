#pragma once

#include "command.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		extern PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel;
		extern PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel;

		struct CommandPoolPrivate : CommandPool
		{
			VkCommandPool vk_command_pool;
			ID3D12CommandAllocator* d3d12_command_allocator = nullptr;

			~CommandPoolPrivate();

			void reset() override;
		};

		struct CommandBufferPrivate : CommandBuffer
		{
			CommandPoolPrivate* pool;

			PipelineType curr_plt = PipelineNone;
			PipelineLayoutPtr curr_pll = nullptr;
			GraphicsPipelinePtr curr_gpl = nullptr;
			ComputePipelinePtr curr_cpl = nullptr;
			FramebufferPtr curr_fb = nullptr;
			RenderpassPtr curr_rp = nullptr;
			int curr_sp = -1;

			VkCommandBuffer vk_command_buffer;
			VkQueryPool vk_query_pool = nullptr;

			ID3D12GraphicsCommandList* d3d12_command_list = nullptr;

			~CommandBufferPrivate();

			void begin(bool once = false);
			void begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs = nullptr) override;
			void next_pass() override;
			void end_renderpass() override;
			void set_viewport(const Rect& rect) override;
			void set_scissor(const Rect& rect) override;
			void bind_pipeline_layout(PipelineLayoutPtr pll, PipelineType plt = PipelineGraphics) override;
			void bind_pipeline(GraphicsPipelinePtr pl) override;
			void bind_pipeline(ComputePipelinePtr pl) override;
			void bind_descriptor_sets(uint idx, std::span<DescriptorSetPtr> dss) override;
			void bind_vertex_buffer(BufferPtr buf, uint id) override;
			void bind_index_buffer(BufferPtr buf, IndiceType t) override;
			void push_constant(uint offset, uint size, const void* data) override;
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void draw_indirect(BufferPtr buf, uint offset, uint count) override;
			void draw_indexed_indirect(BufferPtr buf, uint offset, uint count) override;
			void dispatch(const uvec3& v) override;
			void draw_mesh_tasks(const uvec3& v) override;
			void buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage) override;
			void image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout new_layout,
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone,
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) override;
			void copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies) override;
			void copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies) override;
			void copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies) override;
			void copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies) override;
			void blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter) override;
			void clear_color_image(ImagePtr img, const ImageSub& sub, const vec4& color) override;
			void clear_depth_image(ImagePtr img, const ImageSub& sub, float depth) override;
			void set_event(EventPtr ev) override;
			void reset_event(EventPtr ev) override;
			void wait_event(EventPtr ev) override;
			void begin_debug_label(const std::string& str) override;
			void end_debug_label() override;
			void end() override;

			void calc_executed_time() override;
		};

		struct QueuePrivate : Queue
		{
			VkQueue vk_queue;
			ID3D12CommandQueue* d3d12_queue = nullptr;

			~QueuePrivate();

			void wait_idle() override;
			void submit(std::span<CommandBufferPtr> commandbuffers, std::span<SemaphorePtr> wait_semaphores, std::span<SemaphorePtr> signal_semaphores, FencePtr signal_fence) override;
			void present(std::span<SwapchainPtr> swapchains, std::span<SemaphorePtr> wait_semaphores) override;
		};

		struct SemaphorePrivate : Semaphore
		{
			VkSemaphore vk_semaphore;

			~SemaphorePrivate();
		};

		struct EventPrivate : Event
		{
			VkEvent vk_event;

			~EventPrivate();
		};

		struct FencePrivate : Fence
		{
			uint value = 0;
			VkFence vk_fence = 0;
			ID3D12Fence* d3d12_fence = nullptr;
			HANDLE d3d12_event = 0;

			~FencePrivate();

			void wait(bool auto_reset = true) override;
		};

		extern std::unique_ptr<CommandPoolT> graphics_command_pool;
		extern std::unique_ptr<CommandPoolT> transfer_command_pool;
		extern std::unique_ptr<QueueT> graphics_queue;
		extern std::unique_ptr<QueueT> transfer_queue;
	}
}
