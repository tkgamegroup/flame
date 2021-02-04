#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Image;
		struct Renderpass;
		struct Framebuffer;
		struct Pipeline;
		struct PipelineLayout;
		struct DescriptorSet;
		struct Swapchain;
		struct Semaphore;
		struct Fence;

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
			uvec2 dst_off = uvec2(0);
			uvec2 size;
		};

		struct BufferImageCopy
		{
			uint buffer_offset = 0;
			uvec2 image_offset = uvec2(0);
			uvec2 image_extent;
			uint image_level = 0;
			uint image_base_layer = 0;
			uint image_layer_count = 1;
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

			virtual void begin_renderpass(Renderpass* rp, Framebuffer* fb, const vec4* cvs = nullptr) = 0;
			virtual void end_renderpass() = 0;
			virtual void set_viewport(const Rect& rect) = 0;
			virtual void set_scissor(const Rect& rect) = 0;
			virtual void bind_pipeline_layout(PipelineLayout* pll) = 0;
			virtual void bind_pipeline(Pipeline* pl) = 0;
			virtual void bind_descriptor_set(DescriptorSet* ds, uint idx) = 0;
			virtual void bind_descriptor_set(uint64 h, DescriptorSet* ds) = 0;
			virtual void bind_vertex_buffer(Buffer* buf, uint id) = 0;
			virtual void bind_index_buffer(Buffer* buf, IndiceType t) = 0;
			virtual void push_constant(uint offset, uint size, const void* data) = 0;
			template <class T>
			inline void push_constant_t(uint offset, const T& data)
			{
				push_constant(offset, sizeof(T), &data);
			}
			virtual void push_constant_h(uint64 h, uint size, const void* data) = 0;
			template <class T>
			inline void push_constant_ht(uint64 h, const T& data)
			{
				push_constant_h(h, sizeof(T), &data);
			}
			virtual void draw(uint count, uint instance_count, uint first_vertex, uint first_instance) = 0;
			virtual void draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance) = 0;
			virtual void draw_indirect(Buffer* buf, uint offset, uint count) = 0;
			virtual void draw_indexed_indirect(Buffer* buf, uint offset, uint count) = 0;
			virtual void dispatch(const uvec3& v) = 0;
			virtual void buffer_barrier(Buffer* buf, AccessFlags src_access, AccessFlags dst_access) = 0;
			virtual void image_barrier(Image* img, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, 
				AccessFlags src_access = AccessNone, AccessFlags dst_access = AccessNone) = 0;

			virtual void copy_buffer(Buffer* src, Buffer* dst, uint copies_count, BufferCopy* copies) = 0;
			virtual void copy_image(Image* src, Image* dst, uint copies_count, ImageCopy* copies) = 0;
			virtual void copy_buffer_to_image(Buffer* src, Image* dst, uint copies_count, BufferImageCopy* copies) = 0;
			virtual void copy_image_to_buffer(Image* src, Buffer* dst, uint copies_count, BufferImageCopy* copies) = 0;

			virtual void clear_color_image(Image* img, const cvec4& color) = 0;
			virtual void clear_depth_image(Image* img, float depth) = 0;

			virtual void end() = 0;

			virtual void save(const wchar_t* filename) = 0;

			FLAME_GRAPHICS_EXPORTS  static CommandBuffer* create(CommandPool* p, bool sub = false);
		};

		struct Queue
		{
			virtual void release() = 0;

			virtual void wait_idle() = 0;
			virtual void submit(uint cbs_count, CommandBuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence) = 0;
			virtual void present(Swapchain* s, Semaphore* wait_semaphore) = 0;

			FLAME_GRAPHICS_EXPORTS static Queue* get(Device* device, QueueFamily family = QueueGraphics);
			FLAME_GRAPHICS_EXPORTS static Queue* create(Device* device, uint queue_family_idx);
		};
	}
}
