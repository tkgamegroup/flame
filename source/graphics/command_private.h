#pragma once

#include "command.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
#if USE_D3D12
#elif USE_VULKAN
#endif

		struct CommandPoolPrivate : CommandPool
		{
#if USE_D3D12
			ID3D12CommandAllocator* d3d12_command_allocator = nullptr;
#elif USE_VULKAN
			VkCommandPool vk_command_pool = 0;
#endif

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

#if USE_D3D12
			ID3D12GraphicsCommandList* d3d12_command_list = nullptr;
#elif USE_VULKAN
			VkCommandBuffer vk_command_buffer = 0;
			VkQueryPool vk_query_pool = 0;
#endif

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
			void bind_vertex_buffer(BufferPtr buf, uint id, uint stride) override;
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
#if USE_D3D12
			FencePtr idle_fence = nullptr;
			ID3D12CommandQueue* d3d12_queue = nullptr;
#elif USE_VULKAN
			VkQueue vk_queue = 0;
#endif

			~QueuePrivate();

			void wait_idle() override;
			void submit(std::span<CommandBufferPtr> commandbuffers, std::span<SemaphorePtr> wait_semaphores, std::span<SemaphorePtr> signal_semaphores, FencePtr signal_fence) override;
			void present(std::span<SwapchainPtr> swapchains, std::span<SemaphorePtr> wait_semaphores) override;
		};

		struct SemaphorePrivate : Semaphore
		{
#if USE_D3D12

#elif USE_VULKAN
			VkSemaphore vk_semaphore = 0;
#endif

			~SemaphorePrivate();
		};

		struct EventPrivate : Event
		{
#if USE_D3D12

#elif USE_VULKAN
			VkEvent vk_event = 0;
#endif

			~EventPrivate();
		};

		struct FencePrivate : Fence
		{
#if USE_D3D12
			ID3D12Fence* d3d12_fence = nullptr;
			HANDLE d3d12_event = 0;
#elif USE_VULKAN
			VkFence vk_fence = 0;
#endif
			uint value = 0;

			~FencePrivate();

			void wait(bool auto_reset = true) override;
		};

		extern std::unique_ptr<CommandPoolT> graphics_command_pool;
		extern std::unique_ptr<CommandPoolT> transfer_command_pool;
		extern std::unique_ptr<QueueT> graphics_queue;
		extern std::unique_ptr<QueueT> transfer_queue;
	}
}
