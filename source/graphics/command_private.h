#include <flame/graphics/command.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct CommandPoolPrivate : CommandPool
		{
			DevicePrivate* device;
			VkCommandPool vk_command_buffer_pool;

			CommandPoolPrivate(DevicePrivate* device, int queue_family_idx);
			~CommandPoolPrivate();

			void release() override { delete this; }
		};

		struct CommandBufferPrivate : CommandBuffer
		{
			CommandPoolPrivate* pool;

			PipelineLayoutPrivate* pipeline_layout = nullptr;
			PipelinePrivate* pipeline = nullptr;

			VkCommandBuffer vk_command_buffer;

			CommandBufferPrivate(CommandPoolPrivate* p, bool sub = false);
			~CommandBufferPrivate();

			void release() override { delete this; }

			void begin(bool once = false);
			void begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs = nullptr) override;
			void next_pass() override;
			void end_renderpass() override;
			void set_viewport(const Rect& rect) override;
			void set_scissor(const Rect& rect) override;
			void bind_pipeline_layout(PipelineLayoutPtr pll) override;
			void bind_pipeline(PipelinePtr pl) override;
			void bind_descriptor_sets(uint idx, std::span<DescriptorSetPtr> dss);
			void bind_descriptor_sets(uint idx, uint cnt, DescriptorSetPtr* dss) override { bind_descriptor_sets(idx, { dss, cnt }); }
			void bind_vertex_buffer(BufferPtr buf, uint id) override;
			void bind_index_buffer(BufferPtr buf, IndiceType t) override;
			void push_constant(uint offset, uint size, const void* data) override;
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void draw_indirect(BufferPtr buf, uint offset, uint count) override;
			void draw_indexed_indirect(BufferPtr buf, uint offset, uint count) override;
			void dispatch(const uvec3& v) override;
			void buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access) override;
			void image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout old_layout, ImageLayout new_layout, AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone) override;
			void copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies);
			void copy_buffer(BufferPtr src, BufferPtr dst, uint copies_count, BufferCopy* copies) override { copy_buffer(src, dst, { copies, copies_count }); }
			void copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies);
			void copy_image(ImagePtr src, ImagePtr dst, uint copies_count, ImageCopy* copies) override { copy_image(src, dst, { copies, copies_count }); }
			void copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies);
			void copy_buffer_to_image(BufferPtr src, ImagePtr dst, uint copies_count, BufferImageCopy* copies) override { copy_buffer_to_image(src, dst, { copies, copies_count }); }
			void copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies);
			void copy_image_to_buffer(ImagePtr src, BufferPtr dst, uint copies_count, BufferImageCopy* copies) override { copy_image_to_buffer(src, dst, { copies, copies_count }); }
			void blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter);
			void blit_image(ImagePtr src, ImagePtr dst, uint blits_count, ImageBlit* blits, Filter filter) override { blit_image(src, dst, { blits, blits_count }, filter); }
			void clear_color_image(ImagePtr img, const cvec4& color) override;
			void clear_depth_image(ImagePtr img, float depth) override;
			void end() override;
		};

		struct QueuePrivate : Queue
		{
			DevicePrivate* device;
			VkQueue vk_queue;

			QueuePrivate(DevicePrivate* device, uint queue_family_idx);

			void release() override { delete this; }

			void wait_idle() override;
			void submit(std::span<CommandBufferPtr> cbs, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence);
			void submit(uint cbs_count, CommandBufferPtr* cbs, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence) override { submit({ cbs, cbs_count }, wait_semaphore, signal_semaphore, signal_fence); }
			void present(SwapchainPtr s, SemaphorePtr wait_semaphore) override;
		};
	}
}
