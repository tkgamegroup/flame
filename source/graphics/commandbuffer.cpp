// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "device_private.h"
#include "commandbuffer_private.h"
#include "renderpass_private.h"
#include "framebuffer_private.h"
#include "pipeline_private.h"
#include "descriptor_private.h"
#include "buffer_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		inline CommandpoolPrivate::CommandpoolPrivate(Device *_d, int queue_family_idx)
		{
			d = (DevicePrivate*)_d;

			VkCommandPoolCreateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			info.pNext = nullptr;
			info.queueFamilyIndex = queue_family_idx;

			vk_chk_res(vkCreateCommandPool(d->v, &info, nullptr, &v));
		}

		inline CommandpoolPrivate::~CommandpoolPrivate()
		{
			vkDestroyCommandPool(d->v, v, nullptr);
		}

		Commandpool *Commandpool::create(Device *d, int queue_family_idx)
		{
			return new CommandpoolPrivate(d, queue_family_idx);
		}

		void Commandpool::destroy(Commandpool *p)
		{
			delete (CommandpoolPrivate*)p;
		}

		inline CommandbufferPrivate::CommandbufferPrivate(Commandpool *_p, bool sub)
		{
			p = (CommandpoolPrivate*)_p;
			current_pipeline = nullptr;

			VkCommandBufferAllocateInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			info.commandPool = p->v;
			info.commandBufferCount = 1;

			vk_chk_res(vkAllocateCommandBuffers(p->d->v, &info, &v));
		}

		inline CommandbufferPrivate::~CommandbufferPrivate()
		{
			vkFreeCommandBuffers(p->d->v, p->v, 1, &v);
		}

		inline void CommandbufferPrivate::begin(bool once)
		{
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT |
				(once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0);
			info.pNext = nullptr;
			info.pInheritanceInfo = nullptr;

			vk_chk_res(vkBeginCommandBuffer(v, &info));
			current_pipeline = nullptr;
		}

		inline void CommandbufferPrivate::begin_renderpass(Renderpass *r, Framebuffer *f, ClearValues *cv)
		{
			auto size = ((FramebufferPrivate*)f)->info.views[0]->image()->size;

			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = ((RenderpassPrivate*)r)->v;
			info.framebuffer = ((FramebufferPrivate*)f)->v;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			info.renderArea.extent.width = size.x;
			info.renderArea.extent.height = size.y;
			info.clearValueCount = cv ? ((ClearvaluesPrivate*)cv)->v.size() : 0;
			info.pClearValues = cv ? ((ClearvaluesPrivate*)cv)->v.data() : nullptr;

			vkCmdBeginRenderPass(v, &info, VK_SUBPASS_CONTENTS_INLINE);
		}

		inline void CommandbufferPrivate::end_renderpass()
		{
			vkCmdEndRenderPass(v);
		}

		inline void CommandbufferPrivate::set_viewport(const Rect &rect)
		{
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = rect.min.x;
			vp.y = rect.min.y;
			vp.width = max(rect.width(), 1.f);
			vp.height = max(rect.height(), 1.f);
			vkCmdSetViewport(v, 0, 1, &vp);
		}

		inline void CommandbufferPrivate::set_scissor(const Rect &rect)
		{
			VkRect2D sc;
			sc.offset.x = max(0, rect.min.x);
			sc.offset.y = max(0, rect.min.y);
			sc.extent.width = max(0, rect.width());
			sc.extent.height = max(0, rect.height());
			vkCmdSetScissor(v, 0, 1, &sc);
		}

		inline void CommandbufferPrivate::bind_pipeline(Pipeline *p)
		{
			if (current_pipeline == p)
				return;

			assert(p->type != PipelineNone);
			current_pipeline = (PipelinePrivate*)p;
			vkCmdBindPipeline(v, Z(p->type), ((PipelinePrivate*)p)->v);
		}

		inline void CommandbufferPrivate::bind_descriptorset(Descriptorset *s, int idx)
		{
			vkCmdBindDescriptorSets(v, Z(current_pipeline->type), current_pipeline->layout->v, idx, 1, &((DescriptorsetPrivate*)s)->v, 0, nullptr);
		}

		inline void CommandbufferPrivate::bind_vertexbuffer(Buffer *b, int id)
		{
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(v, id, 1, &((BufferPrivate*)b)->v, &offset);
		}

		inline void CommandbufferPrivate::bind_indexbuffer(Buffer *b, IndiceType t)
		{
			vkCmdBindIndexBuffer(v, ((BufferPrivate*)b)->v, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
		}

		inline void CommandbufferPrivate::push_constant(int offset, int size,  const void *data, Pipelinelayout *layout)
		{
			if (layout == nullptr)
				layout = current_pipeline->layout;
			vkCmdPushConstants(v, ((PipelinelayoutPrivate*)layout)->v, Z(ShaderType(ShaderAll)), offset, size, data);
		}

		inline void CommandbufferPrivate::draw(int count, int instance_count, int first_vertex, int first_instance)
		{
			vkCmdDraw(v, count, instance_count, first_vertex, first_instance);
		}

		inline void CommandbufferPrivate::draw_indexed(int count, int first_index, int vertex_offset, int instance_count, int first_instance)
		{
			vkCmdDrawIndexed(v, count, instance_count, first_index, vertex_offset, first_instance);
		}

		inline void CommandbufferPrivate::dispatch(const Ivec3 &_v)
		{
			vkCmdDispatch(v, _v.x, _v.y, _v.z);
		}

		inline void CommandbufferPrivate::copy_buffer(Buffer *src, Buffer *dst, int copy_count, BufferCopy *copies)
		{
			std::vector<VkBufferCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i].srcOffset = copies[i].src_off;
				vk_copies[i].dstOffset = copies[i].dst_off;
				vk_copies[i].size = copies[i].size;
			}
			vkCmdCopyBuffer(v, ((BufferPrivate*)src)->v, ((BufferPrivate*)dst)->v, copy_count, vk_copies.data());
		}

		inline void CommandbufferPrivate::copy_image(Image *src, Image *dst, int copy_count, ImageCopy *copies)
		{
			std::vector<VkImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
			{
				vk_copies[i].srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				vk_copies[i].srcSubresource.mipLevel = 0;
				vk_copies[i].srcSubresource.baseArrayLayer = 0;
				vk_copies[i].srcSubresource.layerCount = 1;
				vk_copies[i].srcOffset.x = copies[i].src_off.x;
				vk_copies[i].srcOffset.y = copies[i].src_off.y;
				vk_copies[i].srcOffset.z = 0;
				vk_copies[i].dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				vk_copies[i].dstSubresource.mipLevel = 0;
				vk_copies[i].dstSubresource.baseArrayLayer = 0;
				vk_copies[i].dstSubresource.layerCount = 1;
				vk_copies[i].dstOffset.x = copies[i].dst_off.x;
				vk_copies[i].dstOffset.y = copies[i].dst_off.y;
				vk_copies[i].dstOffset.z = 0;
				vk_copies[i].extent.width = copies[i].size.x;
				vk_copies[i].extent.height = copies[i].size.y;
				vk_copies[i].extent.depth = 1;
			}
			vkCmdCopyImage(v, ((ImagePrivate*)src)->v, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				((ImagePrivate*)dst)->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_count, vk_copies.data());
		}

		VkBufferImageCopy to_vk_copy(const BufferImageCopy &cpy, VkImageAspectFlags aspect)
		{
			VkBufferImageCopy vk_cpy = {};
			vk_cpy.bufferOffset = cpy.buffer_offset;
			vk_cpy.imageOffset.x = cpy.image_x;
			vk_cpy.imageOffset.y = cpy.image_y;
			vk_cpy.imageExtent.width = cpy.image_width;
			vk_cpy.imageExtent.height = cpy.image_height;
			vk_cpy.imageExtent.depth = 1;
			vk_cpy.imageSubresource.aspectMask = aspect;
			vk_cpy.imageSubresource.mipLevel = cpy.image_level;
			vk_cpy.imageSubresource.layerCount = 1;
			return vk_cpy;
		}

		inline void CommandbufferPrivate::copy_buffer_to_image(Buffer *src, Image *dst, int copy_count, BufferImageCopy *copies)
		{
			auto aspect = Z(aspect_from_format(dst->format));

			std::vector<VkBufferImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
				vk_copies[i] = to_vk_copy(copies[i], aspect);
			vkCmdCopyBufferToImage(v, ((BufferPrivate*)src)->v, ((ImagePrivate*)dst)->v,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, copy_count, vk_copies.data());
		}

		inline void CommandbufferPrivate::copy_image_to_buffer(Image *src, Buffer *dst, int copy_count, BufferImageCopy *copies)
		{
			auto aspect = Z(aspect_from_format(src->format));

			std::vector<VkBufferImageCopy> vk_copies(copy_count);
			for (auto i = 0; i < copy_count; i++)
				vk_copies[i] = to_vk_copy(copies[i], aspect);
			vkCmdCopyImageToBuffer(v, ((ImagePrivate*)src)->v,
				VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ((BufferPrivate*)dst)->v, copy_count, vk_copies.data());
		}

		inline void CommandbufferPrivate::change_image_layout(Image *i, ImageLayout from, ImageLayout to,
			int base_level, int level_count, int base_layer, int layer_count)
		{
			level_count = level_count == 0 ? i->level : level_count;
			layer_count = layer_count == 0 ? i->layer : layer_count;

			VkImageMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.oldLayout = Z(from, i->format);
			barrier.newLayout = Z(to, i->format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = ((ImagePrivate*)i)->v;
			barrier.subresourceRange.aspectMask = Z(aspect_from_format(i->format));
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
		}

		inline void CommandbufferPrivate::clear_image(ImagePrivate *i, const Bvec4 &col)
		{
			VkClearColorValue cv;
			cv.float32[0] = col.x / 255.f;
			cv.float32[1] = col.y / 255.f;
			cv.float32[2] = col.z / 255.f;
			cv.float32[3] = col.w / 255.f;
			VkImageSubresourceRange r;
			r.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			r.baseMipLevel = 0;
			r.levelCount = 1;
			r.baseArrayLayer = 0;
			r.layerCount = 1;
			vkCmdClearColorImage(v, i->v, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &cv, 1, &r);
		}

		inline void CommandbufferPrivate::end() 
		{
			vk_chk_res(vkEndCommandBuffer(v));
		}

		void Commandbuffer::begin(bool once)
		{
			((CommandbufferPrivate*)this)->begin(once);
		}

		void Commandbuffer::begin_renderpass(Renderpass *r, Framebuffer *f, ClearValues *cv)
		{
			((CommandbufferPrivate*)this)->begin_renderpass(r, f, cv);
		}

		void Commandbuffer::end_renderpass()
		{
			((CommandbufferPrivate*)this)->end_renderpass();
		}

		void Commandbuffer::set_viewport(const Rect &rect)
		{
			((CommandbufferPrivate*)this)->set_viewport(rect);
		}

		void Commandbuffer::set_scissor(const Rect &rect)
		{
			((CommandbufferPrivate*)this)->set_scissor(rect);
		}

		void Commandbuffer::bind_pipeline(Pipeline *p)
		{
			((CommandbufferPrivate*)this)->bind_pipeline(p);
		}

		void Commandbuffer::bind_descriptorset(Descriptorset *s, int idx)
		{
			((CommandbufferPrivate*)this)->bind_descriptorset(s, idx);
		}

		void Commandbuffer::bind_vertexbuffer(Buffer *b, int id)
		{
			((CommandbufferPrivate*)this)->bind_vertexbuffer(b, id);
		}

		void Commandbuffer::bind_indexbuffer(Buffer *b, IndiceType t)
		{
			((CommandbufferPrivate*)this)->bind_indexbuffer(b, t);
		}

		void Commandbuffer::push_constant(int offset, int size, const void *data, Pipelinelayout *layout)
		{
			((CommandbufferPrivate*)this)->push_constant(offset, size, data, layout);
		}

		void Commandbuffer::draw(int count, int instance_count, int first_vertex, int first_instance)
		{
			((CommandbufferPrivate*)this)->draw(count, instance_count, first_vertex, first_instance);
		}

		void Commandbuffer::draw_indexed(int count, int first_index, int vertex_offset, int instance_count, int first_instance)
		{
			((CommandbufferPrivate*)this)->draw_indexed(count, first_index, vertex_offset, instance_count, first_instance);
		}

		void Commandbuffer::dispatch(const Ivec3 &v)
		{
			((CommandbufferPrivate*)this)->dispatch(v);
		}

		void Commandbuffer::copy_buffer(Buffer *src, Buffer *dst, int copy_count, BufferCopy *copies)
		{
			((CommandbufferPrivate*)this)->copy_buffer(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_image(Image *src, Image *dst, int copy_count, ImageCopy *copies)
		{
			((CommandbufferPrivate*)this)->copy_image(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_buffer_to_image(Buffer *src, Image *dst, int copy_count, BufferImageCopy *copies)
		{
			((CommandbufferPrivate*)this)->copy_buffer_to_image(src, dst, copy_count, copies);
		}

		void Commandbuffer::copy_image_to_buffer(Image *src, Buffer *dst, int copy_count, BufferImageCopy *copies)
		{
			((CommandbufferPrivate*)this)->copy_image_to_buffer(src, dst, copy_count, copies);
		}

		void Commandbuffer::change_image_layout(Image *i, ImageLayout from, ImageLayout to,
			int base_level, int level_count, int base_layer, int layer_count)
		{
			((CommandbufferPrivate*)this)->change_image_layout(i, from, to, base_level, level_count, base_layer, layer_count);
		}

		void Commandbuffer::clear_image(Image *i, const Bvec4 &col)
		{
			((CommandbufferPrivate*)this)->clear_image((ImagePrivate*)i, col);
		}

		void Commandbuffer::end()
		{
			((CommandbufferPrivate*)this)->end();
		}

		Commandbuffer* Commandbuffer::create(Commandpool *p, bool sub)
		{
			return new CommandbufferPrivate(p, sub);
		}

		void Commandbuffer::destroy(Commandbuffer *c)
		{
			delete (CommandbufferPrivate*)c;
		}
	}
}
