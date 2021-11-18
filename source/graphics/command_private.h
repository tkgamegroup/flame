#include "command.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct CommandPoolPrivate : CommandPool
		{
			DevicePrivate* device;
			VkCommandPool vk_command_buffer_pool;

			~CommandPoolPrivate();
		};

		struct CommandBufferPrivate : CommandBuffer
		{
			CommandPoolPrivate* pool;

			VkPipelineLayout vk_pll = nullptr;
			VkPipelineBindPoint vk_plt = VK_PIPELINE_BIND_POINT_GRAPHICS;

			VkCommandBuffer vk_command_buffer;

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
			void buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage) override;
			void image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout old_layout, ImageLayout new_layout,
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone,
				PipelineStageFlags src_stage = PipelineStageAllCommand, PipelineStageFlags dst_stage = PipelineStageAllCommand) override;
			void copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies) override;
			void copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies) override;
			void copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies) override;
			void copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies) override;
			void blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter) override;
			void clear_color_image(ImagePtr img, const ImageSub& sub, const cvec4& color) override;
			void clear_depth_image(ImagePtr img, const ImageSub& sub, float depth) override;
			void end() override;
		};

		struct QueuePrivate : Queue
		{
			DevicePrivate* device;
			VkQueue vk_queue;

			void wait_idle() override;
			void submit(std::span<CommandBufferPtr> commandbuffers, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence) override;
			void present(SwapchainPtr s, SemaphorePtr wait_semaphore) override;
		};

		struct SemaphorePrivate : Semaphore
		{
			DevicePrivate* device;
			VkSemaphore vk_semaphore;

			~SemaphorePrivate();
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* device;
			uint value = 0;
			VkFence vk_fence;

			~FencePrivate();

			void wait(bool auto_reset = true) override;
		};
	}
}
