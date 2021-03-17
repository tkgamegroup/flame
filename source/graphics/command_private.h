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

			CommandPoolPrivate(DevicePrivate* device, int queue_family_idx);
			~CommandPoolPrivate();

			void release() override { delete this; }
		};

		struct CommandBufferBridge : CommandBuffer
		{
			void begin_renderpass(Renderpass* rp, Framebuffer* fb, const vec4* clearvalues) override;
			void bind_pipeline_layout(PipelineLayout* pll) override;
			void bind_pipeline(Pipeline* p) override;
			void bind_descriptor_set(DescriptorSet* s, uint idx) override;
			void bind_descriptor_set(uint64 h, DescriptorSet* s) override;
			void bind_vertex_buffer(Buffer* b, uint id) override;
			void bind_index_buffer(Buffer* b, IndiceType t) override;
			void draw_indirect(Buffer* b, uint offset, uint count) override;
			void draw_indexed_indirect(Buffer* b, uint offset, uint count) override;
			void buffer_barrier(Buffer* b, AccessFlags src_access, AccessFlags dst_access) override;
			void image_barrier(Image* i, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, AccessFlags src_access, AccessFlags dst_access) override;
			void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) override;
			void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) override;
			void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) override;
			void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) override;
			void clear_color_image(Image* i, const cvec4& color) override;
			void clear_depth_image(Image* i, float depth) override;

			void save(const wchar_t* filename) override;
		};

		struct CommandBufferPrivate : CommandBufferBridge
		{
			CommandPoolPrivate* pool;

			PipelineLayoutPrivate* pipeline_layout = nullptr;
			PipelinePrivate* pipeline = nullptr;

			VkCommandBuffer vk_command_buffer;

			CommandBufferPrivate(CommandPoolPrivate* p, bool sub = false);
			~CommandBufferPrivate();

			void release() override { delete this; }

			void begin(bool once = false);
			void begin_renderpass(RenderpassPrivate* rp, FramebufferPrivate* fb, const vec4* cvs = nullptr);
			void end_renderpass() override;
			void set_viewport(const Rect& rect) override;
			void set_scissor(const Rect& rect) override;
			void bind_pipeline_layout(PipelineLayoutPrivate* pll);
			void bind_pipeline(PipelinePrivate* pl);
			void bind_descriptor_set(DescriptorSetPrivate* ds, uint idx);
			void bind_descriptor_set(uint64 h, DescriptorSetPrivate* ds);
			void bind_vertex_buffer(BufferPrivate* buf, uint id);
			void bind_index_buffer(BufferPrivate* buf, IndiceType t);
			void push_constant(uint offset, uint size, const void* data) override;
			void push_constant_h(uint64 h, uint size, const void* data);
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void draw_indirect(BufferPrivate* buf, uint offset, uint count);
			void draw_indexed_indirect(BufferPrivate* buf, uint offset, uint count);
			void dispatch(const uvec3& v) override;
			void buffer_barrier(BufferPrivate* buf, AccessFlags src_access, AccessFlags dst_access);
			void image_barrier(ImagePrivate* img, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone);
			void copy_buffer(BufferPrivate* src, BufferPrivate* dst, std::span<BufferCopy> copies);
			void copy_image(ImagePrivate* src, ImagePrivate* dst, std::span<ImageCopy> copies);
			void copy_buffer_to_image(BufferPrivate* src, ImagePrivate* dst, std::span<BufferImageCopy> copies);
			void copy_image_to_buffer(ImagePrivate* src, BufferPrivate* dst, std::span<BufferImageCopy> copies);
			void clear_color_image(ImagePrivate* img, const cvec4& color);
			void clear_depth_image(ImagePrivate* img, float depth);
			void end() override;
		};

		inline void CommandBufferBridge::begin_renderpass(Renderpass* rp, Framebuffer* fb, const vec4* cvs)
		{
			((CommandBufferPrivate*)this)->begin_renderpass((RenderpassPrivate*)rp, (FramebufferPrivate*)fb, cvs);
		}

		inline void CommandBufferBridge::bind_pipeline_layout(PipelineLayout* pll)
		{
			((CommandBufferPrivate*)this)->bind_pipeline_layout((PipelineLayoutPrivate*)pll);
		}

		inline void CommandBufferBridge::bind_pipeline(Pipeline* pl)
		{
			((CommandBufferPrivate*)this)->bind_pipeline((PipelinePrivate*)pl);
		}

		inline void CommandBufferBridge::bind_descriptor_set(DescriptorSet* ds, uint idx)
		{
			((CommandBufferPrivate*)this)->bind_descriptor_set((DescriptorSetPrivate*)ds, idx);
		}

		inline void CommandBufferBridge::bind_descriptor_set(uint64 h, DescriptorSet* ds)
		{
			((CommandBufferPrivate*)this)->bind_descriptor_set(h, (DescriptorSetPrivate*)ds);
		}

		inline void CommandBufferBridge::bind_vertex_buffer(Buffer* buf, uint id)
		{
			((CommandBufferPrivate*)this)->bind_vertex_buffer((BufferPrivate*)buf, id);
		}

		inline void CommandBufferBridge::bind_index_buffer(Buffer* buf, IndiceType t)
		{
			((CommandBufferPrivate*)this)->bind_index_buffer((BufferPrivate*)buf, t);
		}

		inline void CommandBufferBridge::draw_indirect(Buffer* b, uint offset, uint count)
		{
			((CommandBufferPrivate*)this)->draw_indirect((BufferPrivate*)b, offset, count);
		}

		inline void CommandBufferBridge::draw_indexed_indirect(Buffer* b, uint offset, uint count)
		{
			((CommandBufferPrivate*)this)->draw_indexed_indirect((BufferPrivate*)b, offset, count);
		}

		inline void CommandBufferBridge::buffer_barrier(Buffer* b, AccessFlags src_access, AccessFlags dst_access)
		{
			((CommandBufferPrivate*)this)->buffer_barrier((BufferPrivate*)b, src_access, dst_access);
		}

		inline void CommandBufferBridge::image_barrier(Image* i, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, AccessFlags src_access, AccessFlags dst_access)
		{
			((CommandBufferPrivate*)this)->image_barrier((ImagePrivate*)i, subresource, old_layout, new_layout, src_access, dst_access);
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

		inline void CommandBufferBridge::clear_color_image(Image* i, const cvec4& color)
		{
			((CommandBufferPrivate*)this)->clear_color_image((ImagePrivate*)i, color);
		}

		inline void CommandBufferBridge::clear_depth_image(Image* i, float depth)
		{
			((CommandBufferPrivate*)this)->clear_depth_image((ImagePrivate*)i, depth);
		}

		inline void CommandBufferBridge::save(const wchar_t* filename)
		{
			((CommandBufferPrivate*)this)->save(filename);
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

			QueuePrivate(DevicePrivate* device, uint queue_family_idx);

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
