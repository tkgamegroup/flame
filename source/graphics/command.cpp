#include "device_private.h"
#include "renderpass_private.h"
#include "command_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "window_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		std::unique_ptr<CommandPoolT> graphics_command_pool;
		std::unique_ptr<CommandPoolT> transfer_command_pool;
		std::unique_ptr<QueueT> graphics_queue;
		std::unique_ptr<QueueT> transfer_queue;

		CommandPoolPrivate::~CommandPoolPrivate()
		{
			if (app_exiting) return;

			vkDestroyCommandPool(device->vk_device, vk_command_buffer_pool, nullptr);
			unregister_object(vk_command_buffer_pool);
		}

		struct CommandPoolGet : CommandPool::Get
		{
			CommandPoolPtr operator()(QueueFamily family) override
			{
				switch (family)
				{
				case QueueGraphics:
					return graphics_command_pool.get();
				case QueueTransfer:
					return transfer_command_pool.get();
				}
				return nullptr;
			}
		}CommandPool_get;
		CommandPool::Get& CommandPool::get = CommandPool_get;

		struct CommandPoolCreate : CommandPool::Create
		{
			CommandPoolPtr operator()(int queue_family_idx) override
			{
				auto ret = new CommandPoolPrivate;

				VkCommandPoolCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				info.pNext = nullptr;
				info.queueFamilyIndex = queue_family_idx;

				chk_res(vkCreateCommandPool(device->vk_device, &info, nullptr, &ret->vk_command_buffer_pool));
				register_object(ret->vk_command_buffer_pool, "Command Buffer Pool", ret);

				return ret;
			}
		}CommandPool_create;
		CommandPool::Create& CommandPool::create = CommandPool_create;

		CommandBufferPrivate::~CommandBufferPrivate()
		{
			if (app_exiting) return;

			vkFreeCommandBuffers(device->vk_device, pool->vk_command_buffer_pool, 1, &vk_command_buffer);
			vkDestroyQueryPool(device->vk_device, vk_query_pool, nullptr);
			unregister_object(vk_command_buffer);
		}

		void CommandBufferPrivate::begin(bool once)
		{
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.pNext = nullptr;
			info.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			info.pInheritanceInfo = nullptr;

			chk_res(vkBeginCommandBuffer(vk_command_buffer, &info));

			if (want_executed_time)
			{
				if (!vk_query_pool)
				{
					VkQueryPoolCreateInfo info;
					info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
					info.pNext = nullptr;
					info.flags = 0;
					info.queryType = VK_QUERY_TYPE_TIMESTAMP;
					info.queryCount = 2;
					info.pipelineStatistics = 0;
					vkCreateQueryPool(device->vk_device, &info, nullptr, &vk_query_pool);
				}

				vkCmdResetQueryPool(vk_command_buffer, vk_query_pool, 0, 2);
 				vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vk_query_pool, 0);
			}
		}

		void CommandBufferPrivate::begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs)
		{
			curr_fb = fb;
			curr_rp = rp;
			if (!curr_rp)
				curr_rp = curr_fb->renderpass;
			curr_sp = 0;

			for (auto i = 0; i < curr_fb->views.size(); i++)
			{
				auto& att = curr_rp->attachments[i];
				auto layout = att.initia_layout;
				auto iv = curr_fb->views[i];
				auto& sub = iv->sub;
				auto& ly = iv->image->levels[sub.base_level].layers[sub.base_layer];
				if (layout != ImageLayoutUndefined && layout != ly.layout)
					printf("begin renderpass: image layout mismatch, please review your commandbuffer\n");
				ly.layout = att.final_layout;
			}

			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = curr_rp->vk_renderpass;
			info.framebuffer = curr_fb->vk_framebuffer;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			auto first_view = curr_fb->views[0];
			auto size = first_view->image->levels[first_view->sub.base_level].size;
			info.renderArea.extent.width = size.x;
			info.renderArea.extent.height = size.y;
			info.clearValueCount = cvs ? curr_fb->views.size() : 0;
			info.pClearValues = (VkClearValue*)cvs;

			vkCmdBeginRenderPass(vk_command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		void CommandBufferPrivate::next_pass()
		{
			curr_sp++;
			vkCmdNextSubpass(vk_command_buffer, VK_SUBPASS_CONTENTS_INLINE);
		}

		void CommandBufferPrivate::end_renderpass()
		{
			curr_sp = -1;
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

		void CommandBufferPrivate::bind_pipeline_layout(PipelineLayoutPtr pll, PipelineType plt)
		{
			curr_plt = plt;
			curr_pll = pll;
		}

		void CommandBufferPrivate::bind_pipeline(GraphicsPipelinePtr pl)
		{
			if (curr_gpl == pl)
				return;
			curr_plt = PipelineGraphics;
			curr_pll = pl->layout;
			curr_gpl = pl;
			curr_cpl = nullptr;

			auto vk_pl = curr_gpl->vk_pipeline;
			if (curr_gpl->dynamic_renderpass && curr_rp != curr_gpl->renderpass)
				vk_pl = curr_gpl->get_dynamic_pipeline(curr_rp, curr_sp);
			vkCmdBindPipeline(vk_command_buffer, to_backend(curr_plt), vk_pl);
		}

		void CommandBufferPrivate::bind_pipeline(ComputePipelinePtr pl)
		{
			if (curr_cpl == pl)
				return;
			curr_plt = PipelineCompute;
			curr_pll = pl->layout;
			curr_gpl = nullptr;
			curr_cpl = pl;

			vkCmdBindPipeline(vk_command_buffer, to_backend(curr_plt), curr_cpl->vk_pipeline);
		}

		void CommandBufferPrivate::bind_descriptor_sets(uint idx, std::span<DescriptorSetPtr> dss)
		{
			/*
			for (auto ds : dss)
			{
				auto i = 0;
				for (auto& b : ds->layout->bindings)
				{
					if (b.type == DescriptorSampledImage)
					{
						for (auto& r : ds->reses[i])
						{
							auto iv = r.i.p;
							if (iv)
							{
								for (auto ii = 0; ii < iv->sub.level_count; ii++)
								{
									for (auto jj = 0; jj < iv->sub.layer_count; jj++)
									{
										if (iv->image->levels[ii].layers[jj].layout != ImageLayoutShaderReadOnly)
											printf("bind descriptor sets: image is not in shader read only layout\n");
									}
								}
							}
						}
					}
					i++;
				}
			}
			*/

			std::vector<VkDescriptorSet> vk_sets(dss.size());
			auto i = 0;
			for (auto d : dss)
				vk_sets[i++] = d->vk_descriptor_set;
			vkCmdBindDescriptorSets(vk_command_buffer, to_backend(curr_plt), curr_pll->vk_pipeline_layout, idx, vk_sets.size(), vk_sets.data(), 0, nullptr);
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
			vkCmdPushConstants(vk_command_buffer, curr_pll->vk_pipeline_layout, to_backend_flags<ShaderStageFlags>(ShaderStageAll), offset, size, data);
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

		void CommandBufferPrivate::image_barrier(ImagePtr img, const ImageSub& sub, ImageLayout new_layout, 
			AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage)
		{
			ImageLayout old_layout = (ImageLayout)-1;
			for (auto i = 0; i < sub.level_count; i++)
			{
				auto& lv = img->levels[sub.base_level + i];
				for (auto j = 0; j < sub.layer_count; j++)
				{
					auto& ly = lv.layers[sub.base_layer + j];
					if (old_layout == (ImageLayout)-1)
						old_layout = ly.layout;
					else if (ly.layout != old_layout)
						printf("image barrier: image layout mismatch, please review your commandbuffer\n");
					ly.layout = new_layout;
				}
			}
			if (old_layout == new_layout)
				return;

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
				case ImageLayoutGeneral:
					src_access = AccessShaderRead | AccessShaderWrite;
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
				case ImageLayoutGeneral:
					dst_access = AccessShaderRead | AccessShaderWrite;
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

		VkImageCopy to_backend(const ImageCopy& src, VkImageAspectFlags aspect)
		{
			VkImageCopy ret = {};
			ret.srcSubresource.aspectMask = aspect;
			ret.srcSubresource.mipLevel = src.src_sub.base_level;
			ret.srcSubresource.baseArrayLayer = src.src_sub.base_layer;
			ret.srcSubresource.layerCount = src.src_sub.layer_count;
			ret.srcOffset.x = src.src_off.x;
			ret.srcOffset.y = src.src_off.y;
			ret.srcOffset.z = 0;
			ret.dstSubresource.aspectMask = aspect;
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
			ret.dstOffsets[0] = { src.dst_range.x, src.dst_range.y, 0 };
			ret.dstOffsets[1] = { src.dst_range.z, src.dst_range.w, 1 };
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
			auto aspect = to_backend_flags<ImageAspectFlags>(aspect_from_format(src->format));

			std::vector<VkImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_backend(copies[i], aspect);
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
			vkCmdBlitImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
				vk_blits.size(), vk_blits.data(), to_backend(filter));
		}

		void CommandBufferPrivate::clear_color_image(ImagePtr img, const ImageSub& sub, const vec4& color)
		{
			VkClearColorValue cv;
			cv.float32[0] = color.x;
			cv.float32[1] = color.y;
			cv.float32[2] = color.z;
			cv.float32[3] = color.w;
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
			if (want_executed_time)
				vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vk_query_pool, 1);

			chk_res(vkEndCommandBuffer(vk_command_buffer));
		}

		void CommandBufferPrivate::calc_executed_time()
		{
			if (vk_query_pool)
			{
				uint64 timestamps[2];
				vkGetQueryPoolResults(device->vk_device, vk_query_pool, 0, 2, sizeof(uint64) * 2, timestamps, sizeof(uint64), VK_QUERY_RESULT_64_BIT);
				last_executed_time = timestamps[1] - timestamps[0];
			}
		}

		struct CommandBufferCreate : CommandBuffer::Create
		{
			CommandBufferPtr operator()(CommandPoolPtr pool, bool sub) override
			{
				if (!pool)
					pool = CommandPool::get();

				auto ret = new CommandBufferPrivate;
				ret->pool = pool;

				VkCommandBufferAllocateInfo info;
				info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				info.pNext = nullptr;
				info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
				info.commandPool = pool->vk_command_buffer_pool;
				info.commandBufferCount = 1;

				chk_res(vkAllocateCommandBuffers(device->vk_device, &info, &ret->vk_command_buffer));
				register_object(ret->vk_command_buffer, "Command Buffer", ret);

				ret->begin();
				ret->end();

				return ret;
			}
		}CommandBuffer_create;
		CommandBuffer::Create& CommandBuffer::create = CommandBuffer_create;

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

		struct QueueGet : Queue::Get
		{
			QueuePtr operator()(QueueFamily family) override
			{
				switch (family)
				{
				case QueueGraphics:
					return graphics_queue.get();
				case QueueTransfer:
					return transfer_queue.get();
				}
				return nullptr;
			}
		}Queue_get;
		Queue::Get& Queue::get = Queue_get;

		struct QueueCreate : Queue::Create
		{
			QueuePtr operator()(uint queue_family_idx) override
			{
				auto ret = new QueuePrivate;

				vkGetDeviceQueue(device->vk_device, queue_family_idx, 0, &ret->vk_queue);

				return ret;
			}
		}Queue_create;
		Queue::Create& Queue::create = Queue_create;

		SemaphorePrivate::~SemaphorePrivate()
		{
			if (app_exiting) return;

			vkDestroySemaphore(device->vk_device, vk_semaphore, nullptr);
			unregister_object(vk_semaphore);
		}

		struct SemaphoreCreate : Semaphore::Create
		{
			SemaphorePtr operator()() override
			{
				auto ret = new SemaphorePrivate;

				VkSemaphoreCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				chk_res(vkCreateSemaphore(device->vk_device, &info, nullptr, &ret->vk_semaphore));
				register_object(ret->vk_semaphore, "Semaphore", ret);

				return ret;
			}
		}Semaphore_create;
		Semaphore::Create& Semaphore::create = Semaphore_create;

		FencePrivate::~FencePrivate()
		{
			if (app_exiting) return;

			vkDestroyFence(device->vk_device, vk_fence, nullptr);
			unregister_object(vk_fence);
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

		struct FenceCreate : Fence::Create
		{
			FencePtr operator()(bool signaled) override
			{
				auto ret = new FencePrivate;

				VkFenceCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				if (signaled)
				{
					info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
					ret->value = 1;
				}
				chk_res(vkCreateFence(device->vk_device, &info, nullptr, &ret->vk_fence));
				register_object(ret->vk_fence, "Fence", ret);

				return ret;
			}
		}Fence_create;
		Fence::Create& Fence::create = Fence_create;
	}
}
