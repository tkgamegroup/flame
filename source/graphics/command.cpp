 #include "device_private.h"
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

		CommandPool* CommandPool::get(Device* _device, QueueFamily family)
		{
			auto device = (DevicePrivate*)_device;
			switch (family)
			{
			case QueueGraphics:
				return device->gcp.get();
			case QueueTransfer:
				return device->tcp.get();
			}
			return nullptr;
		}

		CommandPool* CommandPool::create(Device* device, int queue_family_idx)
		{
			return new CommandPoolPrivate((DevicePrivate*)device, queue_family_idx);
		}

		CommandBufferPrivate::CommandBufferPrivate(CommandPoolPtr p, bool sub) :
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
			info.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			info.pNext = nullptr;
			info.pInheritanceInfo = nullptr;

			chk_res(vkBeginCommandBuffer(vk_command_buffer, &info));
		}

		void CommandBufferPrivate::begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs)
		{
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = rp ? rp->vk_renderpass : fb->renderpass->vk_renderpass;
			info.framebuffer = fb->vk_framebuffer;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			auto first_view = fb->views[0];
			auto size = first_view->image->sizes[first_view->sub.base_level];
			info.renderArea.extent.width = size.x;
			info.renderArea.extent.height = size.y;
			info.clearValueCount = cvs ? fb->views.size() : 0;
			info.pClearValues = (VkClearValue*)cvs;

			vkCmdBeginRenderPass(vk_command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void CommandBufferPrivate::next_pass()
		{
			vkCmdNextSubpass(vk_command_buffer, VK_SUBPASS_CONTENTS_INLINE);
		}

		void CommandBufferPrivate::end_renderpass()
		{
			vkCmdEndRenderPass(vk_command_buffer);
		}

		void CommandBufferPrivate::set_viewport(const Rect& rect)
		{
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = rect.a.x;
			vp.y = rect.a.y;
			vp.width = max(rect.b.x - rect.a.x, 1.f);
			vp.height = max(rect.b.y - rect.a.y, 1.f);
			vkCmdSetViewport(vk_command_buffer, 0, 1, &vp);
		}

		void CommandBufferPrivate::set_scissor(const Rect& rect)
		{
			VkRect2D sc;
			sc.offset.x = max(0.f, rect.a.x);
			sc.offset.y = max(0.f, rect.a.y);
			sc.extent.width = max(0.f, rect.b.x - rect.a.x);
			sc.extent.height = max(0.f, rect.b.y - rect.a.y);
			vkCmdSetScissor(vk_command_buffer, 0, 1, &sc);
		}

		void CommandBufferPrivate::bind_pipeline_layout(PipelineLayoutPtr pll)
		{
			pipeline_layout = pll;
			pipeline = nullptr;
		}

		void CommandBufferPrivate::bind_pipeline(PipelinePtr pl)
		{
			pipeline_layout = pl->layout;
			pipeline = pl;
			vkCmdBindPipeline(vk_command_buffer, to_backend(pl->type), pl->vk_pipeline);
		}

		void CommandBufferPrivate::bind_descriptor_sets(uint idx, std::span<DescriptorSetPtr> dss)
		{
			std::vector<VkDescriptorSet> vk_sets(dss.size());
			auto i = 0;
			for (auto d : dss)
				vk_sets[i++] = d->vk_descriptor_set;
			vkCmdBindDescriptorSets(vk_command_buffer, to_backend(pipeline ? pipeline->type : PipelineGraphics),
				pipeline_layout->vk_pipeline_layout, idx, vk_sets.size(), vk_sets.data(), 0, nullptr);
		}

		void CommandBufferPrivate::bind_vertex_buffer(BufferPtr buf, uint id)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(vk_command_buffer, id, 1, &buf->vk_buffer, &offset);
		}

		void CommandBufferPrivate::bind_index_buffer(BufferPtr buf, IndiceType t)
		{
			vkCmdBindIndexBuffer(vk_command_buffer, buf->vk_buffer, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		void CommandBufferPrivate::push_constant(uint offset, uint size, const void* data)
		{
			vkCmdPushConstants(vk_command_buffer, pipeline_layout->vk_pipeline_layout,
				to_backend_flags<ShaderStageFlags>(ShaderStageAll), offset, size, data);
		}

		void CommandBufferPrivate::draw(uint count, uint instance_count, uint first_vertex, uint first_instance)
		{
			vkCmdDraw(vk_command_buffer, count, instance_count, first_vertex, first_instance);
		}

		void CommandBufferPrivate::draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance)
		{
			vkCmdDrawIndexed(vk_command_buffer, count, instance_count, first_index, vertex_offset, first_instance);
		}

		void CommandBufferPrivate::draw_indirect(BufferPtr buf, uint offset, uint count)
		{
			vkCmdDrawIndirect(vk_command_buffer, buf->vk_buffer, offset * sizeof(VkDrawIndirectCommand), count, sizeof(VkDrawIndirectCommand));
		}

		void CommandBufferPrivate::draw_indexed_indirect(BufferPtr buf, uint offset, uint count)
		{
			vkCmdDrawIndexedIndirect(vk_command_buffer, buf->vk_buffer, offset * sizeof(VkDrawIndexedIndirectCommand), count, sizeof(VkDrawIndexedIndirectCommand));
		}

		void CommandBufferPrivate::dispatch(const uvec3& v)
		{
			vkCmdDispatch(vk_command_buffer, v.x, v.y, v.z);
		}

		void CommandBufferPrivate::buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage)
		{
			VkBufferMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = to_backend_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_backend_flags<AccessFlags>(dst_access);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.buffer = buf->vk_buffer;
			barrier.offset = 0;
			barrier.size = buf->size;

			if (src_stage == PipelineStageAllCommand)
			{
				switch (src_access)
				{
				case AccessTransferWrite:
					src_stage = PipelineStageTransfer;
					break;
				}
			}

			if (dst_stage == PipelineStageAllCommand)
			{

			}

			vkCmdPipelineBarrier(vk_command_buffer, to_backend_flags<PipelineStageFlags>(src_stage), to_backend_flags<PipelineStageFlags>(dst_stage),
				0, 0, nullptr, 1, &barrier, 0, nullptr);
		}

		void CommandBufferPrivate::image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout old_layout, ImageLayout new_layout, 
			AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage)
		{
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
					if (img->format >= Format_Color_Begin && img->format <= Format_Color_End)
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
					if (img->format >= Format_Color_Begin && img->format <= Format_Color_End)
						dst_access = AccessColorAttachmentWrite;
					else
						dst_access = AccessDepthAttachmentWrite;
					break;
				case ImageLayoutShaderReadOnly:
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

			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = to_backend_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_backend_flags<AccessFlags>(dst_access);
			barrier.oldLayout = to_backend(old_layout, img->format);
			barrier.newLayout = to_backend(new_layout, img->format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = img->vk_image;
			barrier.subresourceRange.aspectMask = to_backend_flags<ImageAspectFlags>(aspect_from_format(img->format));
			barrier.subresourceRange.baseMipLevel = sub.base_level;
			barrier.subresourceRange.levelCount = sub.level_count;
			barrier.subresourceRange.baseArrayLayer = sub.base_layer;
			barrier.subresourceRange.layerCount = sub.layer_count;

			if (src_stage == PipelineStageAllCommand)
			{
				switch (old_layout)
				{
				case ImageLayoutAttachment:
					if (img->format >= Format_Color_Begin && img->format <= Format_Color_End)
						src_stage = PipelineStageColorAttachmentOutput;
					else
						src_stage = PipelineStageEarlyFragTestShader | PipelineStageLateFragTestShader;
					break;
				}
			}

			if (dst_stage == PipelineStageAllCommand)
			{
				switch (new_layout)
				{
				case ImageLayoutAttachment:
					if (img->format >= Format_Color_Begin && img->format <= Format_Color_End)
						dst_stage = PipelineStageColorAttachmentOutput;
					else
						dst_stage = PipelineStageEarlyFragTestShader | PipelineStageLateFragTestShader;
					break;
				case ImageLayoutShaderReadOnly:
					dst_stage = PipelineStageFragShader;
					break;
				}
			}

			vkCmdPipelineBarrier(vk_command_buffer, to_backend_flags<PipelineStageFlags>(src_stage), to_backend_flags<PipelineStageFlags>(dst_stage),
				0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		VkBufferCopy to_backend(const BufferCopy& src)
		{
			VkBufferCopy ret = {};
			ret.srcOffset = src.src_off;
			ret.dstOffset = src.dst_off;
			ret.size = src.size;
			return ret;
		}

		VkImageCopy to_backend(const ImageCopy& src)
		{
			VkImageCopy ret = {};
			ret.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ret.srcSubresource.mipLevel = src.src_sub.base_level;
			ret.srcSubresource.baseArrayLayer = src.src_sub.base_layer;
			ret.srcSubresource.layerCount = src.src_sub.layer_count;
			ret.srcOffset.x = src.src_off.x;
			ret.srcOffset.y = src.src_off.y;
			ret.srcOffset.z = 0;
			ret.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ret.dstSubresource.mipLevel = src.dst_sub.base_level;
			ret.dstSubresource.baseArrayLayer = src.dst_sub.base_layer;
			ret.dstSubresource.layerCount = src.dst_sub.layer_count;
			ret.dstOffset.x = src.dst_off.x;
			ret.dstOffset.y = src.dst_off.y;
			ret.dstOffset.z = 0;
			ret.extent.width = src.size.x;
			ret.extent.height = src.size.y;
			ret.extent.depth = 1;
			return ret;
		}

		VkBufferImageCopy to_backend(const BufferImageCopy& src, VkImageAspectFlags aspect)
		{
			VkBufferImageCopy ret = {};
			ret.bufferOffset = src.buf_off;
			ret.imageOffset.x = src.img_off.x;
			ret.imageOffset.y = src.img_off.y;
			ret.imageExtent.width = src.img_ext.x;
			ret.imageExtent.height = src.img_ext.y;
			ret.imageExtent.depth = 1;
			ret.imageSubresource.aspectMask = aspect;
			ret.imageSubresource.mipLevel = src.img_sub.base_level;
			ret.imageSubresource.baseArrayLayer = src.img_sub.base_layer;
			ret.imageSubresource.layerCount = src.img_sub.layer_count;
			return ret;
		}

		VkImageBlit to_backend(const ImageBlit& src)
		{
			VkImageBlit ret = {};
			ret.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ret.srcSubresource.mipLevel = src.src_sub.base_level;
			ret.srcSubresource.baseArrayLayer = src.src_sub.base_layer;
			ret.srcSubresource.layerCount = src.src_sub.layer_count;
			ret.srcOffsets[0] = { src.src_range.x, src.src_range.y, 0 };
			ret.srcOffsets[1] = { src.src_range.z, src.src_range.w, 1 };
			ret.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ret.dstSubresource.mipLevel = src.dst_sub.base_level;
			ret.dstSubresource.baseArrayLayer = src.dst_sub.base_layer;
			ret.dstSubresource.layerCount = src.dst_sub.layer_count;
			ret.srcOffsets[0] = { src.dst_range.x, src.dst_range.y, 0 };
			ret.srcOffsets[1] = { src.dst_range.z, src.dst_range.w, 1 };
			return ret;
		}

		void CommandBufferPrivate::copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies)
		{
			std::vector<VkBufferCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i]);
			vkCmdCopyBuffer(vk_command_buffer, src->vk_buffer, dst->vk_buffer, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies)
		{
			std::vector<VkImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i]);
			vkCmdCopyImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies)
		{
			auto aspect = to_backend_flags<ImageAspectFlags>(aspect_from_format(dst->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i], aspect);
			vkCmdCopyBufferToImage(vk_command_buffer, src->vk_buffer, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies)
		{
			auto aspect = to_backend_flags<ImageAspectFlags>(aspect_from_format(src->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i], aspect);
			vkCmdCopyImageToBuffer(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_buffer, vk_copies.size(), vk_copies.data());
		}

		void CommandBufferPrivate::blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter)
		{
			std::vector<VkImageBlit> vk_blits(blits.size());
			for (auto i = 0; i < vk_blits.size(); i++)
				vk_blits[i] = to_backend(blits[i]);
			vkCmdBlitImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_blits.size(), vk_blits.data(), to_backend(filter));
		}

		void CommandBufferPrivate::clear_color_image(ImagePtr img, const ImageSub& sub, const cvec4& color)
		{
			VkClearColorValue cv;
			cv.float32[0] = color.x / 255.f;
			cv.float32[1] = color.y / 255.f;
			cv.float32[2] = color.z / 255.f;
			cv.float32[3] = color.w / 255.f;
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			range.baseMipLevel = sub.base_level;
			range.levelCount = sub.level_count;
			range.baseArrayLayer = sub.base_layer;
			range.layerCount = sub.layer_count;
			vkCmdClearColorImage(vk_command_buffer, img->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &range);
		}

		void CommandBufferPrivate::clear_depth_image(ImagePtr img, const ImageSub& sub, float depth)
		{
			VkClearDepthStencilValue cv;
			cv.depth = depth;
			cv.stencil = 0;
			VkImageSubresourceRange range;
			range.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			range.baseMipLevel = sub.base_level;
			range.levelCount = sub.level_count;
			range.baseArrayLayer = sub.base_layer;
			range.layerCount = sub.layer_count;
			vkCmdClearDepthStencilImage(vk_command_buffer, img->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &range);
		}

		void CommandBufferPrivate::end()
		{
			chk_res(vkEndCommandBuffer(vk_command_buffer));
		}

		CommandBuffer* CommandBuffer::create(CommandPool* pool, bool sub)
		{
			return new CommandBufferPrivate((CommandPoolPrivate*)pool, sub);
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

		void QueuePrivate::submit(std::span<CommandBufferPtr> cbs, SemaphorePtr wait_semaphore, SemaphorePtr signal_semaphore, FencePtr signal_fence)
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

		void QueuePrivate::present(SwapchainPtr swapchain, SemaphorePtr wait_semaphore)
		{
			VkPresentInfoKHR info;
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pNext = nullptr;
			info.pResults = nullptr;
			info.waitSemaphoreCount = wait_semaphore ? 1 : 0;
			info.pWaitSemaphores = wait_semaphore ? &wait_semaphore->vk_semaphore : nullptr;
			info.swapchainCount = 1;
			info.pSwapchains = &swapchain->vk_swapchain;
			info.pImageIndices = &swapchain->image_index;
			chk_res(vkQueuePresentKHR(vk_queue, &info));
		}

		Queue* Queue::get(Device* _device, QueueFamily family)
		{
			auto device = (DevicePrivate*)_device;
			switch (family)
			{
			case QueueGraphics:
				return device->gq.get();
			case QueueTransfer:
				return device->tq.get();
			}
			return nullptr;
		}

		Queue* Queue::create(Device* device, uint queue_family_idx)
		{
			return new QueuePrivate((DevicePrivate*)device, queue_family_idx);
		}

		SemaphorePrivate::SemaphorePrivate(DevicePtr device) :
			device(device)
		{
			VkSemaphoreCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			chk_res(vkCreateSemaphore(device->vk_device, &info, nullptr, &vk_semaphore));
		}

		SemaphorePrivate::~SemaphorePrivate()
		{
			vkDestroySemaphore(device->vk_device, vk_semaphore, nullptr);
		}

		Semaphore* Semaphore::create(Device* device)
		{
			return new SemaphorePrivate((DevicePrivate*)device);
		}

		FencePrivate::FencePrivate(DevicePtr device, bool signaled) :
			device(device)
		{
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			if (signaled)
			{
				info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				value = 1;
			}
			chk_res(vkCreateFence(device->vk_device, &info, nullptr, &vk_fence));
		}

		FencePrivate::~FencePrivate()
		{
			vkDestroyFence(device->vk_device, vk_fence, nullptr);
		}

		void FencePrivate::wait(bool auto_reset)
		{
			if (value > 0)
			{
				chk_res(vkWaitForFences(device->vk_device, 1, &vk_fence, true, UINT64_MAX));
				if (auto_reset)
				{
					chk_res(vkResetFences(device->vk_device, 1, &vk_fence));
					value = 0;
				}
			}
		}

		Fence* Fence::create(Device* device, bool signaled)
		{
			return new FencePrivate((DevicePrivate*)device, signaled);
		}
	}
}
