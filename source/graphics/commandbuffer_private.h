#include <flame/graphics/commandbuffer.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct ImagePrivate;
		struct RenderpassPrivate;
		struct FramebufferPrivate;
		struct PipelinePrivate;

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
			void _begin_renderpass(FramebufferPrivate* fb, const std::span<Vec4f>& clearvalues);
			void end_renderpass() override;
			void set_viewport(const Vec4f& rect) override;
			void set_scissor(const Vec4f& rect) override;
			void bind_pipeline(Pipeline* p) override;
			void bind_descriptorset(Descriptorset* s, uint idx, Pipelinelayout* pll) override;
			void bind_vertexbuffer(Buffer* b, uint id) override;
			void bind_indexbuffer(Buffer* b, IndiceType t) override;
			void push_constant(uint offset, uint size, const void* data, Pipelinelayout* pll) override;
			void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) override;
			void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) override;
			void dispatch(const Vec3u& v) override;

			void copy_buffer(Buffer* src, Buffer* dst, uint copy_count, BufferCopy* copies) override;
			void copy_image(Image* src, Image* dst, uint copy_count, ImageCopy* copies) override;
			void copy_buffer_to_image(Buffer* src, Image* dst, uint copy_count, BufferImageCopy* copies) override;
			void copy_image_to_buffer(Image* src, Buffer* dst, uint copy_count, BufferImageCopy* copies) override;
			void change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count) override;

			void clear_image(Image* i, const Vec4c& col) override;

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
			void submit(uint cb_count, Commandbuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence) override;
			void _submit(const std::span<CommandbufferPrivate*>& cbs, SemaphorePrivate* wait_semaphore, SemaphorePrivate* signal_semaphore, FencePrivate* signal_fence);
			void present(Swapchain* s, Semaphore* wait_semaphore) override;
		};
	}
}
