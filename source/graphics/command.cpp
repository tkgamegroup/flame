 #include "device_private.h"
#include "synchronize_private.h"
#include "renderpass_private.h"
#include "swapchain_private.h"
#include "command_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"

namespace flame
{
	namespace graphics
	{
		CommandPoolPrivate::CommandPoolPrivate(DevicePrivate* device, int queue_family_idx) :
			device(device)
		{
			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.pNext = nullptr;
			info.queueFamilyIndex = queue_family_idx;

			chk_res(vkCreateCommandPool(device->vk_device, &info, nullptr, &vk_command_buffer_pool));
		}

		CommandPoolPrivate::~CommandPoolPrivate()
		{
			vkDestroyCommandPool(device->vk_device, vk_command_buffer_pool, nullptr);
		}

		CommandPool* CommandPool::create(Device* device, int queue_family_idx)
		{
			return new CommandPoolPrivate((DevicePrivate*)device, queue_family_idx);
		}

		CommandBufferPrivate::CommandBufferPrivate(CommandPoolPrivate* p, bool sub) :
			pool(p)
		{
			VkCommandBufferAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			info.commandPool = p->vk_command_buffer_pool;
			info.commandBufferCount = 1;

			chk_res(vkAllocateCommandBuffers(p->device->vk_device, &info, &vk_command_buffer));

			begin();
			end();
		}

		CommandBufferPrivate::~CommandBufferPrivate()
		{
			vkFreeCommandBuffers(pool->device->vk_device, pool->vk_command_buffer_pool, 1, &vk_command_buffer);
		}

		void CommandBufferPrivate::begin(bool once)
		{
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
				(once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
			info.pNext = nullptr;
			info.pInheritanceInfo = nullptr;

			chk_res(vkBeginCommandBuffer(vk_command_buffer, &info));
		}

		void CommandBufferPrivate::begin_renderpass(RenderpassPrivate* rp, FramebufferPrivate* fb, const Vec4f* clearvalues)
		{
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = rp ? rp->vk_renderpass : fb->renderpass->vk_renderpass;
			info.framebuffer = fb->vk_framebuffer;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			auto first_view = fb->views[0];
			auto size = first_view->image->sizes[first_view->subresource.base_level];
			info.renderArea.extent.width = size.x();
			info.renderArea.extent.height = size.y();
			info.clearValueCount = clearvalues ? fb->views.size() : 0;
			info.pClearValues = (VkClearValue*)clearvalues;

			vkCmdBeginRenderPass(vk_command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void CommandBufferPrivate::end_renderpass()
		{
			vkCmdEndRenderPass(vk_command_buffer);
		}

		void CommandBufferPrivate::set_viewport(const Vec4f& rect)
		{
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = rect.x();
			vp.y = rect.y();
			vp.width = max(rect.z() - rect.x(), 1.f);
			vp.height = max(rect.w() - rect.y(), 1.f);
			vkCmdSetViewport(vk_command_buffer, 0, 1, &vp);
		}

		void CommandBufferPrivate::set_scissor(const Vec4f& rect)
		{
			VkRect2D sc;
			sc.offset.x = max(0.f, rect.x());
			sc.offset.y = max(0.f, rect.y());
			sc.extent.width = max(0.f, rect.z() - rect.x());
			sc.extent.height = max(0.f, rect.w() - rect.y());
			vkCmdSetScissor(vk_command_buffer, 0, 1, &sc);
		}

		void CommandBufferPrivate::bind_pipeline(PipelinePrivate* p)
		{
			vkCmdBindPipeline(vk_command_buffer, to_backend(p->type), p->vk_pipeline);
		}

		void CommandBufferPrivate::bind_descriptor_set(PipelineType type, DescriptorSetPrivate* s, uint idx, PipelineLayoutPrivate* pll)
		{
			vkCmdBindDescriptorSets(vk_command_buffer, to_backend(type), pll->vk_pipeline_layout, idx, 1, &s->vk_descriptor_set, 0, nullptr);
		}

		void CommandBufferPrivate::bind_vertex_buffer(BufferPrivate* b, uint id)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(vk_command_buffer, id, 1, &b->vk_buffer, &offset);
		}

		void CommandBufferPrivate::bind_index_buffer(BufferPrivate* b, IndiceType t)
		{
			vkCmdBindIndexBuffer(vk_command_buffer, b->vk_buffer, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void CommandBufferPrivate::push_constant(uint offset, uint size, const void* data, PipelineLayoutPrivate* pll)
		{
			vkCmdPushConstants(vk_command_buffer, pll->vk_pipeline_layout, to_backend_flags<ShaderStageFlags>(ShaderStageAll), offset, size, data);
		}

		void CommandBufferPrivate::draw(uint count, uint instance_count, uint first_vertex, uint first_instance)
		{
			vkCmdDraw(vk_command_buffer, count, instance_count, first_vertex, first_instance);
		}

		void CommandBufferPrivate::draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance)
		{
			vkCmdDrawIndexed(vk_command_buffer, count, instance_count, first_index, vertex_offset, first_instance);
		}

		void CommandBufferPrivate::draw_indirect(BufferPrivate* b, uint offset, uint count)
		{
			vkCmdDrawIndirect(vk_command_buffer, b->vk_buffer, offset * sizeof(VkDrawIndirectCommand), count, sizeof(VkDrawIndirectCommand));
		}

		void CommandBufferPrivate::draw_indexed_indirect(BufferPrivate* b, uint offset, uint count)
		{
			vkCmdDrawIndexedIndirect(vk_command_buffer, b->vk_buffer, offset * sizeof(VkDrawIndexedIndirectCommand), count, sizeof(VkDrawIndexedIndirectCommand));
		}

		void CommandBufferPrivate::dispatch(const Vec3u& v)
		{
			vkCmdDispatch(vk_command_buffer, v.x(), v.y(), v.z());
		}

		void CommandBufferPrivate::buffer_barrier(BufferPrivate* b, AccessFlags src_access, AccessFlags dst_access)
		{
			VkBufferMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = to_backend_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_backend_flags<AccessFlags>(dst_access);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = b->vk_buffer;
			barrier.offset = 0;
			barrier.size = b->size;

			vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 1, &barrier, 0, nullptr);
		}

		void CommandBufferPrivate::image_barrier(ImagePrivate* i, const ImageSubresource& subresource, ImageLayout old_layout, ImageLayout new_layout, AccessFlags src_access, AccessFlags dst_access)
		{
			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.oldLayout = to_backend(old_layout, i->format);
			barrier.newLayout = to_backend(new_layout, i->format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = i->vk_image;
			barrier.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(i->format));
			barrier.subresourceRange.baseMipLevel = subresource.base_level;
			barrier.subresourceRange.levelCount = subresource.level_count;
			barrier.subresourceRange.baseArrayLayer = subresource.base_layer;
			barrier.subresourceRange.layerCount = subresource.layer_count;

			if (src_access == AccessNone)
			{
				switch (old_layout)
				{
				case ImageLayoutTransferDst:
					src_access = AccessTransferWrite;
					break;
				case ImageLayoutTransferSrc:
					src_access = AccessTransferRead;
					break;
				case ImageLayoutAttachment:
					if (i->format >= Format_Color_Begin && i->format <= Format_Color_End)
						src_access = AccessColorAttachmentWrite;
					else
						src_access = AccessDepthAttachmentWrite;
					break;
				case ImageLayoutShaderReadOnly:
					src_access = AccessShaderRead;
					break;
				case ImageLayoutShaderStorage:
					src_access = AccessShaderRead | AccessShaderWrite;
					break;
				case ImageLayoutPresent:
					src_access = AccessMemoryRead;
					break;
				}
			}

			if (dst_access == AccessNone)
			{
				switch (new_layout)
				{
				case ImageLayoutTransferDst:
					dst_access = AccessTransferWrite;
					break;
				case ImageLayoutTransferSrc:
					dst_access = AccessTransferRead;
					break;
				case ImageLayoutAttachment:
					if (i->format >= Format_Color_Begin && i->format <= Format_Color_End)
						dst_access = AccessColorAttachmentWrite;
					else
						dst_access = AccessDepthAttachmentWrite;
					break;
				case ImageLayoutShaderReadOnly:
					if (src_access == AccessNone)
						src_access = AccessHostWrite | AccessTransferWrite;
					dst_access = AccessShaderRead;
					break;
				case ImageLayoutShaderStorage:
					dst_access = AccessShaderRead | AccessShaderWrite;
					break;
				case ImageLayoutPresent:
					dst_access = AccessMemoryRead;
					break;
				}
			}

			barrier.srcAccessMask = to_backend_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_backend_flags<AccessFlags>(dst_access);

			vkCmdPipelineBarrier(vk_command_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		void CommandBufferPrivate::copy_buffer(BufferPrivate* src, BufferPrivate* dst, std::span<BufferCopy> copies)
		{
			std::vector<VkBufferCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
			{
				vk_copies[i].srcOffset = copies[i].src_off;
				vk_copies[i].dstOffset = copies[i].dst_off;
				vk_copies[i].size = copies[i].size;
			}
			vkCmdCopyBuffer(vk_command_buffer, src->vk_buffer, dst->vk_buffer, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::copy_image(ImagePrivate* src, ImagePrivate* dst, std::span<ImageCopy> copies)
		{
			std::vector<VkImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
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
			vkCmdCopyImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
		}

		VkBufferImageCopy to_backend(const BufferImageCopy& cpy, VkImageAspectFlags aspect)
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

		void CommandBufferPrivate::copy_buffer_to_image(BufferPrivate* src, ImagePrivate* dst, std::span<BufferImageCopy> copies)
		{
			auto aspect = to_backend_flags<ImageAspectFlags>(aspect_from_format(dst->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i], aspect);
			vkCmdCopyBufferToImage(vk_command_buffer, src->vk_buffer, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::copy_image_to_buffer(ImagePrivate* src, BufferPrivate* dst, std::span<BufferImageCopy> copies)
		{
			auto aspect = to_backend_flags<ImageAspectFlags>(aspect_from_format(src->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i], aspect);
			vkCmdCopyImageToBuffer(vk_command_buffer, src->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_buffer, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::clear_color_image(ImagePrivate* i, const Vec4c& color)
		{
			VkClearColorValue cv;
			cv.float32[0] = color.x() / 255.f;
			cv.float32[1] = color.y() / 255.f;
			cv.float32[2] = color.z() / 255.f;
			cv.float32[3] = color.w() / 255.f;
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;
			vkCmdClearColorImage(vk_command_buffer, i->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &range);
		}

		void CommandBufferPrivate::clear_depth_image(ImagePrivate* i, float depth)
		{
			VkClearDepthStencilValue cv;
			cv.depth = depth;
			cv.stencil = 0;
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			range.baseMipLevel = 0;
			range.levelCount = 1;
			range.baseArrayLayer = 0;
			range.layerCount = 1;
			vkCmdClearDepthStencilImage(vk_command_buffer, i->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &range);
		}

		void CommandBufferPrivate::end()
		{
			chk_res(vkEndCommandBuffer(vk_command_buffer));
		}

		CommandBuffer* CommandBuffer::create(CommandPool* p, bool sub)
		{
			return new CommandBufferPrivate((CommandPoolPrivate*)p, sub);
		}

		QueuePrivate::QueuePrivate(DevicePrivate* device, uint queue_family_idx) :
			device(device)
		{
			vkGetDeviceQueue(device->vk_device, queue_family_idx, 0, &vk_queue);
		}

		void QueuePrivate::wait_idle()
		{
			chk_res(vkQueueWaitIdle(vk_queue));
		}

		void QueuePrivate::submit(std::span<CommandBufferPrivate*> cbs, SemaphorePrivate* wait_semaphore, SemaphorePrivate* signal_semaphore, FencePrivate* signal_fence)
		{
			VkSubmitInfo info;
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = nullptr;
			VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			info.pWaitDstStageMask = &wait_stage;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &wait_semaphore->vk_semaphore : nullptr;
			info.commandBufferCount = cbs.size();
			std::vector<VkCommandBuffer> vk_cbs;
			vk_cbs.resize(cbs.size());
			for (auto i = 0; i < vk_cbs.size(); i++)
				vk_cbs[i] = cbs[i]->vk_command_buffer;
			info.pCommandBuffers = vk_cbs.data();
			info.signalSemaphoreCount = signal_semaphore ? 1 : 0;
			info.pSignalSemaphores = signal_semaphore ? &signal_semaphore->vk_semaphore : nullptr;

			chk_res(vkQueueSubmit(vk_queue, 1, &info, signal_fence ? signal_fence->vk_fence : nullptr));
			if (signal_fence)
				signal_fence->value = 1;
		}

		void QueuePrivate::present(SwapchainPrivate* sc, SemaphorePrivate* wait_semaphore)
		{
			VkPresentInfoKHR info;
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pNext = nullptr;
			info.pResults = nullptr;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &wait_semaphore->vk_semaphore : nullptr;
			info.swapchainCount = 1;
			info.pSwapchains = &sc->vk_swapchain;
			info.pImageIndices = &sc->image_index;
			chk_res(vkQueuePresentKHR(vk_queue, &info));
		}

		Queue* Queue::create(Device* device, uint queue_family_idx)
		{
			return new QueuePrivate((DevicePrivate*)device, queue_family_idx);
		}

		ImmediateCommandBuffer::ImmediateCommandBuffer()
		{
			cb.reset(new CommandBufferPrivate(default_device->gcp.get()));
			cb->begin(true);
		}

		ImmediateCommandBuffer::~ImmediateCommandBuffer()
		{
			cb->end();
			auto q = default_device->gq.get();
			q->submit(SP(cb.get()), nullptr, nullptr, nullptr);
			q->wait_idle();
		}
	}
}
