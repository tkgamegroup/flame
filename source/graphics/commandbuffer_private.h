#include <flame/graphics/commandbuffer.h>
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
		struct DescriptorsetPrivate;
		struct PipelinePrivate;
		struct PipelinelayoutPrivate;
		struct SemaphorePrivate;
		struct FencePrivate;

		struct CommandpoolPrivate : Commandpool
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkCommandPool v;
#elif defined(FLAME_D3D12)
			ID3D12CommandAllocator* v;
#endif

			CommandpoolPrivate(DevicePrivate* d, int queue_family_idx);
			~CommandpoolPrivate();

			void release() override;
		};

		struct CommandbufferPrivate : Commandbuffer
		{
			CommandpoolPrivate* p;
			RenderpassPrivate* current_renderpass;
			uint current_subpass;
			FramebufferPrivate* current_framebuffer;
			PipelinePrivate* current_pipeline;
#if defined(FLAME_VULKAN)
			VkCommandBuffer v;
#elif defined(FLAME_D3D12)
			ID3D12GraphicsCommandList* v;
			bool recording;
#endif

			CommandbufferPrivate(CommandpoolPrivate* p, bool sub = false);
			~CommandbufferPrivate();

			void release() override;

			void begin(bool once = false) override;

			void begin_renderpass(Framebuffer* fb, uint clearvalues_count, const Vec4f* clearvalues) override;
			void _begin_renderpass(FramebufferPrivate* fb, std::span<Vec4f> clearvalues);
			void end_renderpass() override;
			void set_viewport(const Vec4f& rect) override;
			void set_scissor(const Vec4f& rect) override;
			void bind_pipeline(Pipeline* p) override { _bind_pipeline((PipelinePrivate*)p); }
			void _bind_pipeline(PipelinePrivate* p);
			void bind_descriptorset(Descriptorset* s, uint idx, Pipelinelayout* pll) override { _bind_descriptorset((DescriptorsetPrivate*)s, idx, (PipelinelayoutPrivate*)pll); }
			void _bind_descriptorset(DescriptorsetPrivate* s, uint idx, PipelinelayoutPrivate* pll);
			void bind_vertexbuffer(Buffer* b, uint id) override { _bind_vertexbuffer((BufferPrivate*)b, id); }
			void _bind_vertexbuffer(BufferPrivate* b, uint id);
			void bind_indexbuffer(Buffer* b, IndiceType t) override { _bind_indexbuffer((BufferPrivate*)b, t); }
			void _bind_indexbuffer(BufferPrivate* b, IndiceType t);
			void push_constant(uint offset, uint size, const void* data, Pipelinelayout* pll) override { _push_constant(offset, size, data, (PipelinelayoutPrivate*)pll); }
			void _push_constant(uint offset, uint size, const void* data, PipelinelayoutPrivate* pll);
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void dispatch(const Vec3u& v) override;

			void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) override { _copy_buffer((BufferPrivate*)src, (BufferPrivate*)dst, { copies, copies_count }); }
			void _copy_buffer(BufferPrivate* src, BufferPrivate* dst, std::span<BufferCopy> copies);
			void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) override { _copy_image((ImagePrivate*)src, (ImagePrivate*)dst, { copies, copies_count }); }
			void _copy_image(ImagePrivate* src, ImagePrivate* dst, std::span<ImageCopy> copies);
			void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) override { _copy_buffer_to_image((BufferPrivate*)src, (ImagePrivate*)dst, { copies, copies_count }); }
			void _copy_buffer_to_image(BufferPrivate* src, ImagePrivate* dst, std::span<BufferImageCopy> copies);
			void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) override { _copy_image_to_buffer((ImagePrivate*)src, (BufferPrivate*)dst, { copies, copies_count }); }
			void _copy_image_to_buffer(ImagePrivate* src, BufferPrivate* dst, std::span<BufferImageCopy> copies);
			void change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count) override { _change_image_layout((ImagePrivate*)i, from, to, base_level, level_count, base_layer, layer_count); }
			void _change_image_layout(ImagePrivate* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count);

			void clear_image(Image* i, const Vec4c& col) override { _clear_image((ImagePrivate*)i, col); }
			void _clear_image(ImagePrivate* i, const Vec4c& col);

			void end() override;
		};

		struct QueuePrivate : Queue
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkQueue v;
#elif defined(FLAME_D3D12)
			ID3D12CommandQueue* v;
#endif

			QueuePrivate(DevicePrivate* d, uint queue_family_idx);

			void release() override;

			void wait_idle() override;
			void submit(uint cbs_count, Commandbuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence) override;
			void _submit(std::span<CommandbufferPrivate*> cbs, SemaphorePrivate* wait_semaphore, SemaphorePrivate* signal_semaphore, FencePrivate* signal_fence);
			void present(Swapchain* s, Semaphore* wait_semaphore) override;
		};
	}
}
