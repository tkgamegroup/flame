#include <flame/foundation/blueprint.h>
#include "device_private.h"
#include "synchronize_private.h"
#include "renderpass_private.h"
#include "swapchain_private.h"
#include "commandbuffer_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

namespace flame
{
	namespace graphics
	{
		CommandpoolPrivate::CommandpoolPrivate(Device* _d, int queue_family_idx) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.pNext = nullptr;
			info.queueFamilyIndex = queue_family_idx;

			chk_res(vkCreateCommandPool(d->v, &info, nullptr, &v));
#elif defined(FLAME_D3D12)
			auto res = d->v->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));
#endif
		}

		CommandpoolPrivate::~CommandpoolPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyCommandPool(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		Commandpool* Commandpool::create(Device* d, int queue_family_idx)
		{
			return new CommandpoolPrivate(d, queue_family_idx);
		}

		void Commandpool::destroy(Commandpool* p)
		{
			delete (CommandpoolPrivate*)p;
		}

		CommandbufferPrivate::CommandbufferPrivate(Commandpool* _p, bool sub) :
			p((CommandpoolPrivate*)_p)
		{
			current_renderpass = nullptr;
			current_subpass = 0;
			current_framebuffer = nullptr;
			current_pipeline = nullptr;

#if defined(FLAME_VULKAN)
			VkCommandBufferAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			info.commandPool = p->v;
			info.commandBufferCount = 1;

			chk_res(vkAllocateCommandBuffers(p->d->v, &info, &v));
#elif defined(FLAME_D3D12)
			auto res = p->d->v->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, p->v, nullptr, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));
			recording = true;
			end();
#endif

			begin();
			end();
		}

		CommandbufferPrivate::~CommandbufferPrivate()
		{
#if defined(FLAME_VULKAN)
			vkFreeCommandBuffers(p->d->v, p->v, 1, &v);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::begin(bool once)
		{
#if defined(FLAME_VULKAN)
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
				(once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
			info.pNext = nullptr;
			info.pInheritanceInfo = nullptr;

			chk_res(vkBeginCommandBuffer(v, &info));
#elif defined(FLAME_D3D12)
			if (recording)
				return;
			v->Reset(p->v, nullptr);
			recording = true;
#endif
			current_renderpass = nullptr;
			current_subpass = 0;
			current_framebuffer = nullptr;
			current_pipeline = nullptr;
		}

		void CommandbufferPrivate::begin_renderpass(Framebuffer* _f, Clearvalues* _cv)
		{
			auto r = (RenderpassPrivate*)_f->renderpass();
			auto f = (FramebufferPrivate*)_f;
			auto cv = (ClearvaluesPrivate*)_cv;

			current_renderpass = r;
			current_subpass = 0;
			current_framebuffer = f;

#if defined(FLAME_VULKAN)
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = r->v;
			info.framebuffer = f->v;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			info.renderArea.extent.width = f->image_size.x();
			info.renderArea.extent.height = f->image_size.y();
			info.clearValueCount = cv ? cv->v.size() : 0;
			info.pClearValues = cv ? cv->v.data() : nullptr;

			vkCmdBeginRenderPass(v, &info, VK_SUBPASS_CONTENTS_INLINE);
#elif defined(FLAME_D3D12)
			auto& attachments = r->info.attachments;
			auto& subpass = r->info.subpasses[current_subpass];
			auto& views = f->info.views;
			for (auto& idx : subpass.color_attachments)
			{
				auto& a = attachments[idx];
				auto view = (ImageviewPrivate*)views[idx];
				auto layout_from = ImageLayoutUndefined;
				if (a.format >= Format_Swapchain_Begin && a.format <= Format_Swapchain_End)
					layout_from = ImageLayoutPresent;
				change_image_layout(view->i, layout_from, ImageLayoutAttachment);
				auto descriptor = view->v->GetCPUDescriptorHandleForHeapStart();
				v->OMSetRenderTargets(1, &descriptor, false, nullptr);
				if (a.clear)
					v->ClearRenderTargetView(descriptor, &cv->v[idx].x(), 0, nullptr);
			}
#endif
		}

		void CommandbufferPrivate::end_renderpass()
		{
#if defined(FLAME_VULKAN)
			vkCmdEndRenderPass(v);
#elif defined(FLAME_D3D12)
			auto& attachments = current_renderpass->info.attachments;
			auto& subpass = current_renderpass->info.subpasses[current_subpass];
			auto& views = current_framebuffer->info.views;
			for (auto& idx : subpass.color_attachments)
			{
				auto& a = attachments[idx];
				auto view = (ImageviewPrivate*)views[idx];
				auto layout_to = ImageLayoutUndefined;
				if (a.format >= Format_Swapchain_Begin && a.format <= Format_Swapchain_End)
					layout_to = ImageLayoutPresent;
				change_image_layout(view->i, ImageLayoutAttachment, layout_to);
			}
#endif
		}

		void CommandbufferPrivate::set_viewport(const Vec4f& rect)
		{
#if defined(FLAME_VULKAN)
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = rect.x();
			vp.y = rect.y();
			vp.width = max(rect.z() - rect.x(), 1.f);
			vp.height = max(rect.w() - rect.y(), 1.f);
			vkCmdSetViewport(v, 0, 1, &vp);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::set_scissor(const Vec4f& rect)
		{
#if defined(FLAME_VULKAN)
			VkRect2D sc;
			sc.offset.x = max(0.f, rect.x());
			sc.offset.y = max(0.f, rect.y());
			sc.extent.width = max(0.f, rect.z() - rect.x());
			sc.extent.height = max(0.f, rect.w() - rect.y());
			vkCmdSetScissor(v, 0, 1, &sc);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::bind_pipeline(Pipeline* p)
		{
			if (current_pipeline == p)
				return;

			assert(p->type != PipelineNone);
			current_pipeline = (PipelinePrivate*)p;
#if defined(FLAME_VULKAN)
			vkCmdBindPipeline(v, to_enum(p->type), ((PipelinePrivate*)p)->v);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::bind_descriptorset(Descriptorset* s, uint idx, Pipelinelayout* pll)
		{
#if defined(FLAME_VULKAN)
			vkCmdBindDescriptorSets(v, to_enum(pll ? PipelineGraphics : current_pipeline->type), pll ? ((PipelinelayoutPrivate*)pll)->v : current_pipeline->pll->v, idx, 1, &((DescriptorsetPrivate*)s)->v, 0, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::bind_vertexbuffer(Buffer* b, uint id)
		{
#if defined(FLAME_VULKAN)
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(v, id, 1, &((BufferPrivate*)b)->v, &offset);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::bind_indexbuffer(Buffer* b, IndiceType t)
		{
#if defined(FLAME_VULKAN)
			vkCmdBindIndexBuffer(v, ((BufferPrivate*)b)->v, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::push_constant(uint offset, uint size, const void* data, Pipelinelayout* pll)
		{
			if (!pll)
				pll = current_pipeline->pll;
#if defined(FLAME_VULKAN)
			vkCmdPushConstants(v, pll ? ((PipelinelayoutPrivate*)pll)->v : current_pipeline->pll->v, to_flags(ShaderStageAll), offset, size, data);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::draw(uint count, uint instance_count, uint first_vertex, uint first_instance)
		{
#if defined(FLAME_VULKAN)
			vkCmdDraw(v, count, instance_count, first_vertex, first_instance);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance)
		{
#if defined(FLAME_VULKAN)
			vkCmdDrawIndexed(v, count, instance_count, first_index, vertex_offset, first_instance);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::dispatch(const Vec3u& _v)
		{
#if defined(FLAME_VULKAN)
			vkCmdDispatch(v, _v.x(), _v.y(), _v.z());
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::copy_buffer(Buffer* src, Buffer* dst, uint copy_count, BufferCopy* copies)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkBufferCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i].srcOffset = copies[i].src_off;
				vk_copies[i].dstOffset = copies[i].dst_off;
				vk_copies[i].size = copies[i].size;
			}
			vkCmdCopyBuffer(v, ((BufferPrivate*)src)->v, ((BufferPrivate*)dst)->v, copy_count, vk_copies.data());
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::copy_image(Image* src, Image* dst, uint copy_count, ImageCopy* copies)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				vk_copies[i].srcSubresource.mipLevel = 0;
				vk_copies[i].srcSubresource.baseArrayLayer = 0;
				vk_copies[i].srcSubresource.layerCount = 1;
				vk_copies[i].srcOffset.x = copies[i].src_off.x();
				vk_copies[i].srcOffset.y = copies[i].src_off.y();
				vk_copies[i].srcOffset.z = 0;
				vk_copies[i].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				vk_copies[i].dstSubresource.mipLevel = 0;
				vk_copies[i].dstSubresource.baseArrayLayer = 0;
				vk_copies[i].dstSubresource.layerCount = 1;
				vk_copies[i].dstOffset.x = copies[i].dst_off.x();
				vk_copies[i].dstOffset.y = copies[i].dst_off.y();
				vk_copies[i].dstOffset.z = 0;
				vk_copies[i].extent.width = copies[i].size.x();
				vk_copies[i].extent.height = copies[i].size.y();
				vk_copies[i].extent.depth = 1;
			}
			vkCmdCopyImage(v, ((ImagePrivate*)src)->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				((ImagePrivate*)dst)->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_count, vk_copies.data());
#elif defined(FLAME_D3D12)

#endif
		}

#if defined(FLAME_VULKAN)
		VkBufferImageCopy to_vk_copy(const BufferImageCopy& cpy, VkImageAspectFlags aspect)
		{
			VkBufferImageCopy vk_cpy = {};
			vk_cpy.bufferOffset = cpy.buffer_offset;
			vk_cpy.imageOffset.x = cpy.image_offset.x();
			vk_cpy.imageOffset.y = cpy.image_offset.y();
			vk_cpy.imageExtent.width = cpy.image_extent.x();
			vk_cpy.imageExtent.height = cpy.image_extent.y();
			vk_cpy.imageExtent.depth = 1;
			vk_cpy.imageSubresource.aspectMask = aspect;
			vk_cpy.imageSubresource.mipLevel = cpy.image_level;
			vk_cpy.imageSubresource.layerCount = 1;
			return vk_cpy;
		}
#endif

		void CommandbufferPrivate::copy_buffer_to_image(Buffer* src, Image* dst, uint copy_count, BufferImageCopy* copies)
		{
#if defined(FLAME_VULKAN)
			auto aspect = to_flags(aspect_from_format(dst->format));

			std::vector<VkBufferImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
				vk_copies[i] = to_vk_copy(copies[i], aspect);
			vkCmdCopyBufferToImage(v, ((BufferPrivate*)src)->v, ((ImagePrivate*)dst)->v,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_count, vk_copies.data());
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::copy_image_to_buffer(Image* src, Buffer* dst, uint copy_count, BufferImageCopy* copies)
		{
#if defined(FLAME_VULKAN)
			auto aspect = to_flags(aspect_from_format(src->format));

			std::vector<VkBufferImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
				vk_copies[i] = to_vk_copy(copies[i], aspect);
			vkCmdCopyImageToBuffer(v, ((ImagePrivate*)src)->v,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ((BufferPrivate*)dst)->v, copy_count, vk_copies.data());
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count)
		{
			level_count = level_count == 0 ? i->level : level_count;
			layer_count = layer_count == 0 ? i->layer : layer_count;

#if defined(FLAME_VULKAN)
			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.oldLayout = to_enum(from, i->format);
			barrier.newLayout = to_enum(to, i->format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = ((ImagePrivate*)i)->v;
			barrier.subresourceRange.aspectMask = to_flags(aspect_from_format(i->format));
			barrier.subresourceRange.baseMipLevel = base_level;
			barrier.subresourceRange.levelCount = level_count;
			barrier.subresourceRange.baseArrayLayer = base_layer;
			barrier.subresourceRange.layerCount = layer_count;

			switch (barrier.oldLayout)
			{
			case VK_IMAGE_LAYOUT_UNDEFINED:
				barrier.srcAccessMask = 0;
				break;
			case VK_IMAGE_LAYOUT_GENERAL:
				barrier.srcAccessMask = 0;
				break;
			case VK_IMAGE_LAYOUT_PREINITIALIZED:
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
				barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				break;

			default:
				barrier.srcAccessMask = 0;
				break;
			}

			switch (barrier.newLayout)
			{
			case VK_IMAGE_LAYOUT_GENERAL:
				barrier.dstAccessMask = 0;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				break;
			case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
				barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
				break;
			case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
				break;
			case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
				if (barrier.srcAccessMask == 0)
					barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				break;

			default:
				barrier.dstAccessMask = 0;
				break;
			}

			vkCmdPipelineBarrier(v, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
#elif defined(FLAME_D3D12)
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = ((ImagePrivate*)i)->v;
			barrier.Transition.StateBefore = Z(from);
			barrier.Transition.StateAfter = Z(to);
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
#endif
		}

		void CommandbufferPrivate::clear_image(ImagePrivate* i, const Vec4c& col)
		{
#if defined(FLAME_VULKAN)
			VkClearColorValue cv;
			cv.float32[0] = col.x() / 255.f;
			cv.float32[1] = col.y() / 255.f;
			cv.float32[2] = col.z() / 255.f;
			cv.float32[3] = col.w() / 255.f;
			VkImageSubresourceRange r;
			r.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			r.baseMipLevel = 0;
			r.levelCount = 1;
			r.baseArrayLayer = 0;
			r.layerCount = 1;
			vkCmdClearColorImage(v, i->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &r);
#elif defined(FLAME_D3D12)

#endif
		}

		void CommandbufferPrivate::end()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkEndCommandBuffer(v));
#elif defined(FLAME_D3D12)
			if (recording)
			{
				auto res = v->Close();
				assert(SUCCEEDED(res));
				recording = false;
			}
#endif
		}

		void Commandbuffer::begin(bool once)
		{
			((CommandbufferPrivate*)this)->begin(once);
		}

		void Commandbuffer::begin_renderpass(Framebuffer* f, Clearvalues* cv)
		{
			((CommandbufferPrivate*)this)->begin_renderpass(f, cv);
		}

		void Commandbuffer::end_renderpass()
		{
			((CommandbufferPrivate*)this)->end_renderpass();
		}

		void Commandbuffer::set_viewport(const Vec4f& rect)
		{
			((CommandbufferPrivate*)this)->set_viewport(rect);
		}

		void Commandbuffer::set_scissor(const Vec4f& rect)
		{
			((CommandbufferPrivate*)this)->set_scissor(rect);
		}

		void Commandbuffer::bind_pipeline(Pipeline* p)
		{
			((CommandbufferPrivate*)this)->bind_pipeline(p);
		}

		void Commandbuffer::bind_descriptorset(Descriptorset* s, uint idx, Pipelinelayout* pll)
		{
			((CommandbufferPrivate*)this)->bind_descriptorset(s, idx, pll);
		}

		void Commandbuffer::bind_vertexbuffer(Buffer* b, uint id)
		{
			((CommandbufferPrivate*)this)->bind_vertexbuffer(b, id);
		}

		void Commandbuffer::bind_indexbuffer(Buffer* b, IndiceType t)
		{
			((CommandbufferPrivate*)this)->bind_indexbuffer(b, t);
		}

		void Commandbuffer::push_constant(uint offset, uint size, const void* data, Pipelinelayout* pll)
		{
			((CommandbufferPrivate*)this)->push_constant(offset, size, data, pll);
		}

		void Commandbuffer::draw(uint count, uint instance_count, uint first_vertex, uint first_instance)
		{
			((CommandbufferPrivate*)this)->draw(count, instance_count, first_vertex, first_instance);
		}

		void Commandbuffer::draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance)
		{
			((CommandbufferPrivate*)this)->draw_indexed(count, first_index, vertex_offset, instance_count, first_instance);
		}

		void Commandbuffer::dispatch(const Vec3u& v)
		{
			((CommandbufferPrivate*)this)->dispatch(v);
		}

		void Commandbuffer::copy_buffer(Buffer* src, Buffer* dst, uint copy_count, BufferCopy* copies)
		{
			((CommandbufferPrivate*)this)->copy_buffer(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_image(Image* src, Image* dst, uint copy_count, ImageCopy* copies)
		{
			((CommandbufferPrivate*)this)->copy_image(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_buffer_to_image(Buffer* src, Image* dst, uint copy_count, BufferImageCopy* copies)
		{
			((CommandbufferPrivate*)this)->copy_buffer_to_image(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_image_to_buffer(Image* src, Buffer* dst, uint copy_count, BufferImageCopy* copies)
		{
			((CommandbufferPrivate*)this)->copy_image_to_buffer(src, dst, copy_count, copies);
		}

		void Commandbuffer::change_image_layout(Image* i, ImageLayout from, ImageLayout to, uint base_level, uint level_count, uint base_layer, uint layer_count)
		{
			((CommandbufferPrivate*)this)->change_image_layout(i, from, to, base_level, level_count, base_layer, layer_count);
		}

		void Commandbuffer::clear_image(Image* i, const Vec4c& col)
		{
			((CommandbufferPrivate*)this)->clear_image((ImagePrivate*)i, col);
		}

		void Commandbuffer::end()
		{
			((CommandbufferPrivate*)this)->end();
		}

		Commandbuffer* Commandbuffer::create(Commandpool* p, bool sub)
		{
			return new CommandbufferPrivate(p, sub);
		}

		void Commandbuffer::destroy(Commandbuffer* c)
		{
			delete (CommandbufferPrivate*)c;
		}

		struct Commandbuffer$
		{
			AttributeP<Commandbuffer> out$o;

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (out$o.frame == -1)
				{
					auto d = Device::default_one();
					if (d)
						out$o.v = Commandbuffer::create(d->gcp);
					else
					{
						printf("cannot create commandbuffer\n");

						out$o.v = nullptr;
					}
					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Commandbuffer$()
			{
				if (out$o.v)
					Commandbuffer::destroy((Commandbuffer*)out$o.v);
			}

		};

		struct Commandbuffers$
		{
			AttributeD<uint> size$i;

			AttributeD<Array<Commandbuffer*>> out$o;

			FLAME_GRAPHICS_EXPORTS void update$(BP* scene)
			{
				if (size$i.frame > out$o.frame)
				{
					for (auto i = 0; i < out$o.v.s; i++)
						Commandbuffer::destroy((Commandbuffer*)out$o.v[i]);
					auto d = Device::default_one();
					if (d && size$i.v > 0)
					{
						out$o.v.resize(size$i.v);
						for (auto i = 0; i < size$i.v; i++)
							out$o.v[i] = Commandbuffer::create(d->gcp);
					}
					else
					{
						printf("cannot create commandbuffers\n");

						out$o.v.resize(0);
					}
					out$o.frame = scene->frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Commandbuffers$()
			{
				for (auto i = 0; i < out$o.v.s; i++)
					Commandbuffer::destroy((Commandbuffer*)out$o.v.v[i]);
			}
		};

		QueuePrivate::QueuePrivate(Device* _d, uint queue_family_idx)
		{
			d = (DevicePrivate*)_d;
#if defined(FLAME_VULKAN)
			vkGetDeviceQueue(d->v, queue_family_idx, 0, &v);
#elif defined(FLAME_D3D12)
			D3D12_COMMAND_QUEUE_DESC desc = {};
			auto res = d->v->CreateCommandQueue(&desc, IID_PPV_ARGS(&v));
			assert(SUCCEEDED(res));
#endif
		}

		QueuePrivate::~QueuePrivate()
		{
		}

		void QueuePrivate::wait_idle()
		{
#if defined(FLAME_VULKAN)
			chk_res(vkQueueWaitIdle(v));
#elif defined(FLAME_D3D12)

#endif
		}

		void QueuePrivate::submit(uint cb_count, Commandbuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence)
		{
#if defined(FLAME_VULKAN)
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			info.pWaitDstStageMask = &wait_stage;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &((SemaphorePrivate*)wait_semaphore)->v : nullptr;
			info.commandBufferCount = cb_count;
			std::vector<VkCommandBuffer> vk_cbs;
			vk_cbs.resize(cb_count);
			for (auto i = 0; i < cb_count; i++)
				vk_cbs[i] = ((CommandbufferPrivate*)cbs[i])->v;
			info.pCommandBuffers = vk_cbs.data();
			info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
			info.pSignalSemaphores = signal_semaphore ? &((SemaphorePrivate*)signal_semaphore)->v : nullptr;

			chk_res(vkQueueSubmit(v, 1, &info, signal_fence ? ((FencePrivate*)signal_fence)->v : nullptr));
			if (signal_fence)
				signal_fence->vl = 1;
#elif defined(FLAME_D3D12)
			ID3D12CommandList* list[] = { ((CommandbufferPrivate*)c)->v };
			v->ExecuteCommandLists(1, list);

			if (signal_fence)
			{
				auto fence = (FencePrivate*)signal_fence;
				fence->vl++;
				auto res = v->Signal(fence->v, fence->vl);
				assert(SUCCEEDED(res));
			}
#endif
		}

		void QueuePrivate::present(Swapchain* s, Semaphore* wait_semaphore)
		{
#if defined(FLAME_VULKAN)
			VkPresentInfoKHR info;
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pNext = nullptr;
			info.pResults = nullptr;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &((SemaphorePrivate*)wait_semaphore)->v : nullptr;
			info.swapchainCount = 1;
			info.pSwapchains = &((SwapchainPrivate*)s)->v;
			auto index = s->image_index();
			info.pImageIndices = &index;
			chk_res(vkQueuePresentKHR(v, &info));
#elif defined(FLAME_D3D12)
			auto res = ((SwapchainPrivate*)s)->v->Present(0, 0);
			assert(SUCCEEDED(res));
#endif
		}

		void Queue::wait_idle()
		{
			((QueuePrivate*)this)->wait_idle();
		}

		void Queue::submit(uint cb_count, Commandbuffer* const* cbs, Semaphore* wait_semaphore, Semaphore* signal_semaphore, Fence* signal_fence)
		{
			((QueuePrivate*)this)->submit(cb_count, cbs, wait_semaphore, signal_semaphore, signal_fence);
		}

		void Queue::present(Swapchain* s, Semaphore* wait_semaphore)
		{
			((QueuePrivate*)this)->present(s, wait_semaphore);
		}

		Queue* Queue::create(Device* d, uint queue_family_idx)
		{
			return new QueuePrivate(d, queue_family_idx);
		}

		void Queue::destroy(Queue* q)
		{
			delete (QueuePrivate*)q;
		}
	}
}
