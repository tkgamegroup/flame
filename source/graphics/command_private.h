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
			void bind_pipeline(Pipeline* p) override;
			void bind_descriptor_set(DescriptorSet* s, uint idx) override;
			void bind_vertex_buffer(Buffer* b, uint id) override;
			void bind_index_buffer(Buffer* b, IndiceType t) override;
			void push_constant(uint offset, uint size, const void* data) override;
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
			struct Resource
			{
				uint64 id;

				virtual ~Resource() {}
			};

			struct BufferResource : Resource
			{
			};

			struct ResourceDatabase
			{

			};

			struct Cmd
			{
				virtual ~Cmd() {}

				virtual void save(pugi::xml_node n_tree) = 0;
			};

			struct CmdBeginRenderpass : Cmd
			{
				RenderpassPrivate* rp;
				FramebufferPrivate* fb;
				std::vector<vec4> cvs;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("begin_renderpass");
					n.append_attribute("rp").set_value((uint64)rp);
					n.append_attribute("fb").set_value((uint64)fb);
					auto n_cvs = n.append_child("cvs");
					for (auto& cv : cvs)
						n_cvs.append_child("cv").append_attribute("v").set_value(to_string(cv).c_str());
				}
			};

			struct CmdEndRenderpass : Cmd
			{

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("end_renderpass");

				}
			};

			struct CmdSetViewport : Cmd
			{
				Rect rect;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("set_viewport");
					n.append_attribute("rect").set_value(to_string(rect).c_str());
				}
			};

			struct CmdSetScissor : Cmd
			{
				Rect rect;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("set_scissor");
					n.append_attribute("rect").set_value(to_string(rect).c_str());

				}
			};

			struct CmdBindPipeline : Cmd
			{
				PipelinePrivate* pl;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("bind_pipeline");
					n.append_attribute("pl").set_value((uint64)pl);
				}
			};

			struct CmdBindDescriptorSet : Cmd
			{
				DescriptorSetPrivate* ds;
				uint idx;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("bind_descriptor_set");
					n.append_attribute("ds").set_value((uint64)ds);
					n.append_attribute("idx").set_value(idx);

				}
			};

			struct CmdBindVertexBuffer : Cmd
			{
				BufferPrivate* buf;
				uint id;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("bind_vertex_buffer");
					n.append_attribute("buf").set_value((uint64)buf);
					n.append_attribute("id").set_value(id);

				}
			};

			struct CmdBindIndexBuffer : Cmd
			{
				BufferPrivate* buf;
				IndiceType t;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("bind_index_buffer");
					n.append_attribute("buf").set_value((uint64)buf);
					n.append_attribute("t").set_value((int)t);

				}
			};

			struct CmdPushConstant : Cmd
			{
				uint offset;
				std::string data;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("push_constant");
					n.append_attribute("offset").set_value(offset);
					n.append_attribute("data").set_value(base64::encode((char*)data.data(), data.size()).c_str());
				}
			};

			struct CmdDraw : Cmd
			{
				uint count;
				uint instance_count;
				uint first_vertex;
				uint first_instance;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("draw");
					n.append_attribute("count").set_value(count);
					n.append_attribute("instance_count").set_value(instance_count);
					n.append_attribute("first_vertex").set_value(first_vertex);
					n.append_attribute("first_instance").set_value(first_instance);
				}
			};

			struct CmdDrawIndexed : Cmd
			{
				uint count;
				uint first_index;
				int vertex_offset;
				uint instance_count;
				uint first_instance;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("draw_indexed");
					n.append_attribute("count").set_value(count);
					n.append_attribute("first_index").set_value(first_index);
					n.append_attribute("vertex_offset").set_value(vertex_offset);
					n.append_attribute("instance_count").set_value(instance_count);
					n.append_attribute("first_instance").set_value(first_instance);
				}
			};

			struct CmdDrawIndirect : Cmd
			{
				BufferPrivate* buf;
				uint offset;
				uint count;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("draw_indirect");
					n.append_attribute("buf").set_value((uint64)buf);
					n.append_attribute("offset").set_value(offset);
					n.append_attribute("count").set_value(count);
				}
			};

			struct CmdDrawIndexedIndirect : Cmd
			{
				BufferPrivate* buf;
				uint offset;
				uint count;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("draw_indexed_indirect");
					n.append_attribute("buf").set_value((uint64)buf);
					n.append_attribute("offset").set_value(offset);
					n.append_attribute("count").set_value(count);
				}
			};

			struct CmdDispatch : Cmd
			{
				uvec3 v;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("dispatch");
					n.append_attribute("v").set_value(to_string(v).c_str());
				}
			};

			struct CmdBufferBarrier : Cmd
			{
				BufferPrivate* buf;
				AccessFlags src_access;
				AccessFlags dst_access;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("buffer_barrier");
					n.append_attribute("buf").set_value((uint64)buf);
					n.append_attribute("src_access").set_value((int)src_access);
					n.append_attribute("dst_access").set_value((int)dst_access);
				}
			};

			struct CmdImageBarrier : Cmd
			{
				ImagePrivate* img;
				ImageSubresource subresource;
				ImageLayout old_layout;
				ImageLayout new_layout;
				AccessFlags src_access;
				AccessFlags dst_access;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("image_barrier");
					auto n_sr = n.append_child("subresource");
					n_sr.append_attribute("base_level").set_value(subresource.base_level);
					n_sr.append_attribute("level_count").set_value(subresource.level_count);
					n_sr.append_attribute("base_layer").set_value(subresource.base_layer);
					n_sr.append_attribute("layer_count").set_value(subresource.layer_count);
					n.append_attribute("old_layout").set_value((int)old_layout);
					n.append_attribute("new_layout").set_value((int)new_layout);
					n.append_attribute("src_access").set_value((int)src_access);
					n.append_attribute("dst_access").set_value((int)dst_access);
				}
			};

			struct CmdCopyBuffer : Cmd
			{
				BufferPrivate* src;
				BufferPrivate* dst;
				std::vector<BufferCopy> copies;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("copy_buffer");
					n.append_attribute("src").set_value((uint64)src);
					n.append_attribute("dst").set_value((uint64)dst);
					auto n_copies = n.append_child("copies");
					for (auto& cpy : copies)
					{
						auto n_copy = n_copies.append_child("copy");
						n_copy.append_attribute("src_off").set_value(cpy.src_off);
						n_copy.append_attribute("dst_off").set_value(cpy.dst_off);
						n_copy.append_attribute("size").set_value(cpy.size);
					}
				}
			};

			struct CmdCopyImage : Cmd
			{
				ImagePrivate* src;
				ImagePrivate* dst;
				std::vector<ImageCopy> copies;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("copy_image");
					n.append_attribute("src").set_value((uint64)src);
					n.append_attribute("dst").set_value((uint64)dst);
					auto n_copies = n.append_child("copies");
					for (auto& cpy : copies)
					{
						auto n_copy = n_copies.append_child("copy");
						n_copy.append_attribute("src_off").set_value(to_string(cpy.src_off).c_str());
						n_copy.append_attribute("dst_off").set_value(to_string(cpy.dst_off).c_str());
						n_copy.append_attribute("size").set_value(to_string(cpy.size).c_str());
					}
				}
			};

			struct CmdCopyBufferToImage : Cmd
			{
				BufferPrivate* src;
				ImagePrivate* dst;
				std::vector<BufferImageCopy> copies;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("copy_buffer_to_image");
					n.append_attribute("src").set_value((uint64)src);
					n.append_attribute("dst").set_value((uint64)dst);
					auto n_copies = n.append_child("copies");
					for (auto& cpy : copies)
					{
						auto n_copy = n_copies.append_child("copy");
						n_copy.append_attribute("buffer_offset").set_value(cpy.buffer_offset);
						n_copy.append_attribute("image_offset").set_value(to_string(cpy.image_offset).c_str());
						n_copy.append_attribute("image_extent").set_value(to_string(cpy.image_extent).c_str());
						n_copy.append_attribute("image_level").set_value(cpy.image_level);
					}
				}
			};

			struct CmdCopyImageToBuffer : Cmd
			{
				ImagePrivate* src;
				BufferPrivate* dst;
				std::vector<BufferImageCopy> copies;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("copy_image_to_buffer");
					n.append_attribute("src").set_value((uint64)src);
					n.append_attribute("dst").set_value((uint64)dst);
					auto n_copies = n.append_child("copies");
					for (auto& cpy : copies)
					{
						auto n_copy = n_copies.append_child("copy");
						n_copy.append_attribute("buffer_offset").set_value(cpy.buffer_offset);
						n_copy.append_attribute("image_offset").set_value(to_string(cpy.image_offset).c_str());
						n_copy.append_attribute("image_extent").set_value(to_string(cpy.image_extent).c_str());
						n_copy.append_attribute("image_level").set_value(cpy.image_level);
					}
				}
			};

			struct CmdClearColorImage : Cmd
			{
				ImagePrivate* img;
				cvec4 color;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("clear_color_image");
					n.append_attribute("img").set_value((uint64)img);
					n.append_attribute("color").set_value(to_string(color).c_str());
				}
			};

			struct CmdClearDepthImage : Cmd
			{
				ImagePrivate* img;
				float depth;

				void save(pugi::xml_node n_tree) override
				{
					auto n = n_tree.append_child("clear_depth_image");
					n.append_attribute("img").set_value((uint64)img);
					n.append_attribute("depth").set_value(to_string(depth).c_str());
				}
			};

			CommandPoolPrivate* pool;

			bool record = false;
			std::vector<std::unique_ptr<Cmd>> cmds;

			PipelinePrivate* pipeline = nullptr;

			VkCommandBuffer vk_command_buffer;

			CommandBufferPrivate(CommandPoolPrivate* p, bool sub = false);
			~CommandBufferPrivate();

			void release() override { delete this; }

			void begin(bool once = false, bool record = false);
			void begin_renderpass(RenderpassPrivate* rp, FramebufferPrivate* fb, const vec4* cvs = nullptr);
			void end_renderpass() override;
			void set_viewport(const Rect& rect) override;
			void set_scissor(const Rect& rect) override;
			void bind_pipeline(PipelinePrivate* pl);
			void bind_descriptor_set(DescriptorSetPrivate* ds, uint idx);
			void bind_vertex_buffer(BufferPrivate* buf, uint id);
			void bind_index_buffer(BufferPrivate* buf, IndiceType t);
			void push_constant(uint offset, uint size, const void* data);
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

			void save(const std::filesystem::path& filename);
		};

		inline void CommandBufferBridge::begin_renderpass(Renderpass* rp, Framebuffer* fb, const vec4* cvs)
		{
			((CommandBufferPrivate*)this)->begin_renderpass((RenderpassPrivate*)rp, (FramebufferPrivate*)fb, cvs);
		}

		inline void CommandBufferBridge::bind_pipeline(Pipeline* pp)
		{
			((CommandBufferPrivate*)this)->bind_pipeline((PipelinePrivate*)pp);
		}

		inline void CommandBufferBridge::bind_descriptor_set(DescriptorSet* ds, uint idx)
		{
			((CommandBufferPrivate*)this)->bind_descriptor_set((DescriptorSetPrivate*)ds, idx);
		}

		inline void CommandBufferBridge::bind_vertex_buffer(Buffer* buf, uint id)
		{
			((CommandBufferPrivate*)this)->bind_vertex_buffer((BufferPrivate*)buf, id);
		}

		inline void CommandBufferBridge::bind_index_buffer(Buffer* buf, IndiceType t)
		{
			((CommandBufferPrivate*)this)->bind_index_buffer((BufferPrivate*)buf, t);
		}

		inline void CommandBufferBridge::push_constant(uint offset, uint size, const void* data)
		{
			((CommandBufferPrivate*)this)->push_constant(offset, size, data);
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

		struct ImmediateCommandBuffer
		{
			DevicePrivate* d;

			std::unique_ptr<CommandBufferPrivate> cb;

			ImmediateCommandBuffer(DevicePrivate* d);
			~ImmediateCommandBuffer();
		};
	}
}
