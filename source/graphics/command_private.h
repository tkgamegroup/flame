#include <flame/graphics/command.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct BufferPrivate;
		struct ImagePrivate;
		struct RenderpassPrivate;
		struct FramebufferPrivate;
		struct DescriptorSetPrivate;
		struct PipelinePrivate;
		struct PipelineLayoutPrivate;
		struct SwapchainPrivate;
		struct SemaphorePrivate;
		struct FencePrivate;

		struct CommandPoolPrivate : CommandPool
		{
			DevicePrivate* device;
			VkCommandPool vk_command_buffer_pool;

			CommandPoolPrivate(DevicePrivate* d, int queue_family_idx);
			~CommandPoolPrivate();

			void release() override { delete this; }
		};

		struct CommandBufferBridge : CommandBuffer
		{
			void begin_renderpass(Framebuffer* fb, const Vec4f* clearvalues) override;
			void bind_pipeline(Pipeline* p) override;
			void bind_descriptor_set(DescriptorSet* s, uint idx, PipelineLayout* pll) override;
			void bind_vertex_buffer(Buffer* b, uint id) override;
			void bind_index_buffer(Buffer* b, IndiceType t) override;
			void push_constant(uint offset, uint size, const void* data, PipelineLayout* pll) override;
			void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) override;
			void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) override;
			void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) override;
			void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) override;
			void change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count) override;
			void clear_image(Image* i, const Vec4c& col) override;
		};

		struct CommandBufferPrivate : CommandBufferBridge
		{
			CommandPoolPrivate* command_buffer_pool;

			RenderpassPrivate* current_renderpass = nullptr;
			uint current_subpass = 0;
			FramebufferPrivate* current_framebuffer = nullptr;
			PipelinePrivate* current_pipeline = nullptr;

			VkCommandBuffer vk_command_buffer;

			CommandBufferPrivate(CommandPoolPrivate* p, bool sub = false);
			~CommandBufferPrivate();

			void release() override { delete this; }

			void begin(bool once = false);
			void begin_renderpass(FramebufferPrivate* fb, const Vec4f* clearvalues = nullptr);
			void end_renderpass() override;
			void set_viewport(const Vec4f& rect) override;
			void set_scissor(const Vec4f& rect) override;
			void bind_pipeline(PipelinePrivate* p);
			void bind_descriptor_set(DescriptorSetPrivate* s, uint idx, PipelineLayoutPrivate* pll);
			void bind_vertex_buffer(BufferPrivate* b, uint id);
			void bind_index_buffer(BufferPrivate* b, IndiceType t);
			void push_constant(uint offset, uint size, const void* data, PipelineLayoutPrivate* pll);
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void dispatch(const Vec3u& v) override;
			void copy_buffer(BufferPrivate* src, BufferPrivate* dst, std::span<BufferCopy> copies);
			void copy_image(ImagePrivate* src, ImagePrivate* dst, std::span<ImageCopy> copies);
			void copy_buffer_to_image(BufferPrivate* src, ImagePrivate* dst, std::span<BufferImageCopy> copies);
			void copy_image_to_buffer(ImagePrivate* src, BufferPrivate* dst, std::span<BufferImageCopy> copies);
			void change_image_layout(ImagePrivate* i, ImageLayout from, ImageLayout to, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1);
			void clear_image(ImagePrivate* i, const Vec4c& col);
			void end() override;
		};

		inline void CommandBufferBridge::begin_renderpass(Framebuffer* fb, const Vec4f* clearvalues)
		{
			((CommandBufferPrivate*)this)->begin_renderpass((FramebufferPrivate*)fb, clearvalues);
		}

		inline void CommandBufferBridge::bind_pipeline(Pipeline* p)
		{
			((CommandBufferPrivate*)this)->bind_pipeline((PipelinePrivate*)p);
		}

		inline void CommandBufferBridge::bind_descriptor_set(DescriptorSet* s, uint idx, PipelineLayout* pll)
		{
			((CommandBufferPrivate*)this)->bind_descriptor_set((DescriptorSetPrivate*)s, idx, (PipelineLayoutPrivate*)pll);
		}

		inline void CommandBufferBridge::bind_vertex_buffer(Buffer* b, uint id)
		{
			((CommandBufferPrivate*)this)->bind_vertex_buffer((BufferPrivate*)b, id);
		}

		inline void CommandBufferBridge::bind_index_buffer(Buffer* b, IndiceType t)
		{
			((CommandBufferPrivate*)this)->bind_index_buffer((BufferPrivate*)b, t);
		}

		inline void CommandBufferBridge::push_constant(uint offset, uint size, const void* data, PipelineLayout* pll)
		{
			((CommandBufferPrivate*)this)->push_constant(offset, size, data, (PipelineLayoutPrivate*)pll);
		}

		inline void CommandBufferBridge::copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies)
		{
			((CommandBufferPrivate*)this)->copy_buffer((BufferPrivate*)src, (BufferPrivate*)dst, { copies, copies_count });
		}

		inline void CommandBufferBridge::copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies)
		{
			((CommandBufferPrivate*)this)->copy_image((ImagePrivate*)src, (ImagePrivate*)dst, { copies, copies_count });
		}

		inline void CommandBufferBridge::copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies)
		{
			((CommandBufferPrivate*)this)->copy_buffer_to_image((BufferPrivate*)src, (ImagePrivate*)dst, { copies, copies_count });
		}

		inline void CommandBufferBridge::copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies)
		{
			((CommandBufferPrivate*)this)->copy_image_to_buffer((ImagePrivate*)src, (BufferPrivate*)dst, { copies, copies_count });
		}

		inline void CommandBufferBridge::change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count)
		{
			((CommandBufferPrivate*)this)->change_image_layout((ImagePrivate*)i, from, to, base_level, level_count, base_layer, layer_count);
		}

		inline void CommandBufferBridge::clear_image(Image* i, const Vec4c& col)
		{
			((CommandBufferPrivate*)this)->clear_image((ImagePrivate*)i, col);
		}

		struct QueueBridge : Queue
		{
			void submit(uint cbs_count, CommandBuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence) override;
			void present(Swapchain* s, Semaphore* wait_semaphore) override;
		};

		struct QueuePrivate : QueueBridge
		{
			DevicePrivate* device;
			VkQueue vk_queue;

			QueuePrivate(DevicePrivate* d, uint queue_family_idx);

			void release() override { delete this; }

			void wait_idle() override;
			void submit(std::span<CommandBufferPrivate*> cbs, SemaphorePrivate* wait_semaphore, SemaphorePrivate* signal_semaphore, FencePrivate* signal_fence);
			void present(SwapchainPrivate* s, SemaphorePrivate* wait_semaphore);
		};

		inline void QueueBridge::submit(uint cbs_count, CommandBuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence)
		{
			((QueuePrivate*)this)->submit({ (CommandBufferPrivate**)cbs, cbs_count }, (SemaphorePrivate*)wait_semaphore, (SemaphorePrivate*)signal_semaphore, (FencePrivate*)signal_fence);
		}

		inline void QueueBridge::present(Swapchain* s, Semaphore* wait_semaphore)
		{
			((QueuePrivate*)this)->present((SwapchainPrivate*)s, (SemaphorePrivate*)wait_semaphore);
		}
	}
}
