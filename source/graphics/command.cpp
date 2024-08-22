#include "device_private.h"
#include "renderpass_private.h"
#include "command_private.h"
#include "buffer_private.h"
#include "image_private.h"
#include "shader_private.h"
#include "window_private.h"
#include "auxiliary.h"

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

#if USE_D3D12
			d3d12_command_allocator->Release();
			unregister_object(d3d12_command_allocator);
#elif USE_VULKAN
			vkDestroyCommandPool(device->vk_device, vk_command_pool, nullptr);
			unregister_object(vk_command_pool);
#endif
		}

		void CommandPoolPrivate::reset()
		{
#if USE_D3D12
			d3d12_command_allocator->Reset();
#elif USE_VULKAN
			vkResetCommandPool(device->vk_device, vk_command_pool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
#endif
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
#if USE_D3D12
				device->d3d12_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&ret->d3d12_command_allocator));
				register_object(ret->d3d12_command_allocator, "Command Buffer Pool", ret);
#elif USE_VULKAN
				VkCommandPoolCreateInfo info;
				info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				info.pNext = nullptr;
				info.queueFamilyIndex = queue_family_idx;

				check_vk_result(vkCreateCommandPool(device->vk_device, &info, nullptr, &ret->vk_command_pool));
				register_object(ret->vk_command_pool, "Command Buffer Pool", ret);
#endif

				return ret;
			}
		}CommandPool_create;
		CommandPool::Create& CommandPool::create = CommandPool_create;

		CommandBufferPrivate::~CommandBufferPrivate()
		{
			if (app_exiting) return;

#if USE_D3D12
			d3d12_command_list->Release();
			unregister_object(d3d12_command_list);
#elif USE_VULKAN
			vkFreeCommandBuffers(device->vk_device, pool->vk_command_pool, 1, &vk_command_buffer);
			vkDestroyQueryPool(device->vk_device, vk_query_pool, nullptr);
			unregister_object(vk_command_buffer);
#endif
		}

		void CommandBufferPrivate::begin(bool once)
		{
			curr_plt = PipelineNone;
			curr_pll = nullptr;
			curr_gpl = nullptr;
			curr_cpl = nullptr;
			curr_fb = nullptr;
			curr_rp = nullptr;
			curr_sp = -1;

#if USE_D3D12
			check_dx_result(d3d12_command_list->Reset(pool->d3d12_command_allocator, nullptr));
			auto descriptor_pool = DescriptorPool::current();
			ID3D12DescriptorHeap* heap_list[2];
			heap_list[0] = descriptor_pool->d3d12_srv_heap;
			heap_list[1] = descriptor_pool->d3d12_sp_heap;
			d3d12_command_list->SetDescriptorHeaps(2, heap_list);
#elif USE_VULKAN
			VkCommandBufferBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			info.pNext = nullptr;
			info.flags = once ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			info.pInheritanceInfo = nullptr;

			check_vk_result(vkBeginCommandBuffer(vk_command_buffer, &info));

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
#endif
		}

		void CommandBufferPrivate::begin_renderpass(RenderpassPtr rp, FramebufferPtr fb, const vec4* cvs)
		{
			curr_fb = fb;
			curr_rp = rp;
			if (!curr_rp)
				curr_rp = curr_fb->renderpass;
			curr_sp = 0;

#if USE_D3D12
			d3d12_command_list->OMSetRenderTargets(curr_fb->d3d12_rtv_num, curr_fb->d3d12_rtv_off != -1 ? &curr_fb->d3d12_rtv_cpu_handle : nullptr, true,
				curr_fb->d3d12_dsv_off != -1 ? &curr_fb->d3d12_dsv_cpu_handle : nullptr);
#endif

			auto rt_idx = 0;
			for (auto i = 0; i < curr_fb->views.size(); i++)
			{
				auto& att = curr_rp->attachments[i];
				auto layout = att.initia_layout;
				auto iv = curr_fb->views[i];
				auto img = iv->image;
				auto& sub = iv->sub;
				auto& ly = iv->image->levels[sub.base_level].layers[sub.base_layer];
				if (layout != ImageLayoutUndefined && layout != ly.layout)
					printf("begin renderpass: image layout mismatch, please review your commandbuffer\n");

				auto old_state = to_dx(ly.layout, img->format);
				auto new_state = to_dx(ImageLayoutAttachment, img->format);
				if (new_state != old_state)
				{
					D3D12_RESOURCE_BARRIER barrier;
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = img->d3d12_resource;
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = old_state;
					barrier.Transition.StateAfter = new_state;
					d3d12_command_list->ResourceBarrier(1, &barrier);
				}

				if (att.format >= Format_Depth_Begin && att.format <= Format_Depth_End)
				{
					if (att.load_op == AttachmentLoadClear)
					{
						D3D12_CLEAR_FLAGS flags = D3D12_CLEAR_FLAG_DEPTH;
						if (att.format >= Format_DepthStencil_Begin && att.format <= Format_DepthStencil_End)
							flags |= D3D12_CLEAR_FLAG_STENCIL;
						d3d12_command_list->ClearDepthStencilView(curr_fb->d3d12_dsv_cpu_handle, flags, cvs[i].x, cvs[i].y, 0, nullptr);
					}
				}
				else
				{
					if (att.load_op == AttachmentLoadClear)
					{
						auto handle = curr_fb->d3d12_rtv_cpu_handle;
						handle.ptr += rt_idx * device->d3d12_rtv_size;
						d3d12_command_list->ClearRenderTargetView(handle, &cvs[i][0], 0, nullptr);
					}
					rt_idx++;
				}

				ly.layout = att.initia_layout;
			}

#if USE_VULKAN
			VkRenderPassBeginInfo info;
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			info.pNext = nullptr;
			info.renderPass = curr_rp->vk_renderpass;
			info.framebuffer = curr_fb->vk_framebuffer;
			info.renderArea.offset.x = 0;
			info.renderArea.offset.y = 0;
			auto first_view = curr_fb->views[0];
			auto ext = first_view->image->levels[first_view->sub.base_level].extent;
			info.renderArea.extent.width = ext.x;
			info.renderArea.extent.height = ext.y;
			info.clearValueCount = cvs ? curr_fb->views.size() : 0;
			info.pClearValues = (VkClearValue*)cvs;

			vkCmdBeginRenderPass(vk_command_buffer, &info, VK_SUBPASS_CONTENTS_INLINE);
#endif
		}

		void CommandBufferPrivate::next_pass()
		{
			curr_sp++;
#if USE_VULKAN
			vkCmdNextSubpass(vk_command_buffer, VK_SUBPASS_CONTENTS_INLINE);
#endif
		}

		void CommandBufferPrivate::end_renderpass()
		{
			for (auto i = 0; i < curr_fb->views.size(); i++)
			{
				auto& att = curr_rp->attachments[i];
				auto layout = att.final_layout;
				auto iv = curr_fb->views[i];
				auto img = iv->image;
				auto& sub = iv->sub;
				auto& ly = iv->image->levels[sub.base_level].layers[sub.base_layer];

#if USE_D3D12
				auto old_state = to_dx(ImageLayoutAttachment, img->format);
				auto new_state = to_dx(att.final_layout, img->format);
				if (new_state != old_state)
				{
					D3D12_RESOURCE_BARRIER barrier;
					barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
					barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
					barrier.Transition.pResource = img->d3d12_resource;
					barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
					barrier.Transition.StateBefore = old_state;
					barrier.Transition.StateAfter = new_state;
					d3d12_command_list->ResourceBarrier(1, &barrier);
				}
#endif

				ly.layout = att.final_layout;
			}

			curr_sp = -1;
#if USE_VULKAN
			vkCmdEndRenderPass(vk_command_buffer);
#endif
		}

		void CommandBufferPrivate::set_viewport(const Rect& rect)
		{
#if USE_D3D12
			D3D12_VIEWPORT vp;
			vp.MinDepth = 0.f;
			vp.MaxDepth = 1.f;
			vp.TopLeftX = rect.a.x;
			vp.TopLeftY = rect.a.y;
			vp.Width = max(rect.b.x - rect.a.x, 1.f);
			vp.Height = max(rect.b.y - rect.a.y, 1.f);
			d3d12_command_list->RSSetViewports(1, &vp);
#elif USE_VULKAN
			VkViewport vp;
			vp.minDepth = 0.f;
			vp.maxDepth = 1.f;
			vp.x = rect.a.x;
			vp.y = rect.a.y;
			vp.width = max(rect.b.x - rect.a.x, 1.f);
			vp.height = max(rect.b.y - rect.a.y, 1.f);
			vkCmdSetViewport(vk_command_buffer, 0, 1, &vp);
#endif
		}

		void CommandBufferPrivate::set_scissor(const Rect& rect)
		{
#if USE_D3D12
			D3D12_RECT sc;
			sc.left = max(0.f, rect.a.x);
			sc.top = max(0.f, rect.a.y);
			sc.right = max(0.f, rect.b.x);
			sc.bottom = max(0.f, rect.b.y);
			d3d12_command_list->RSSetScissorRects(1, &sc);
#elif USE_VULKAN
			VkRect2D sc;
			sc.offset.x = max(0.f, rect.a.x);
			sc.offset.y = max(0.f, rect.a.y);
			sc.extent.width = max(0.f, rect.b.x - rect.a.x);
			sc.extent.height = max(0.f, rect.b.y - rect.a.y);
			vkCmdSetScissor(vk_command_buffer, 0, 1, &sc);
#endif
		}

		void CommandBufferPrivate::bind_pipeline_layout(PipelineLayoutPtr pll, PipelineType plt)
		{
			if (curr_gpl && curr_gpl->layout != pll)
				curr_gpl = nullptr;
			if (curr_cpl && curr_cpl->layout != pll)
				curr_cpl = nullptr;
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

#if USE_D3D12
			d3d12_command_list->SetGraphicsRootSignature(pl->layout->d3d12_root_signature);
			d3d12_command_list->SetPipelineState(pl->d3d12_pipeline);
			D3D12_PRIMITIVE_TOPOLOGY pt = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			switch (curr_gpl->primitive_topology)
			{
			case PrimitiveTopologyTriangleList: 
				pt = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST; 
				break;
			}
			d3d12_command_list->IASetPrimitiveTopology(pt);
#elif USE_VULKAN
			auto vk_pl = curr_gpl->vk_pipeline;
			if (curr_gpl->dynamic_renderpass && curr_rp != curr_gpl->renderpass)
				vk_pl = curr_gpl->get_dynamic_pipeline(curr_rp, curr_sp);
			vkCmdBindPipeline(vk_command_buffer, to_vk(curr_plt), vk_pl);
#endif
		}

		void CommandBufferPrivate::bind_pipeline(ComputePipelinePtr pl)
		{
			if (curr_cpl == pl)
				return;
			curr_plt = PipelineCompute;
			curr_pll = pl->layout;
			curr_gpl = nullptr;
			curr_cpl = pl;

#if USE_D3D12
			d3d12_command_list->SetComputeRootSignature(pl->layout->d3d12_root_signature);
			d3d12_command_list->SetPipelineState(pl->d3d12_pipeline);
#elif USE_VULKAN
			vkCmdBindPipeline(vk_command_buffer, to_vk(curr_plt), curr_cpl->vk_pipeline);
#endif
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

#if USE_D3D12
			for (auto i = 0; i < dss.size(); i++)
			{
				auto id = (i + idx) * 2;
				if (dss[i]->d3d12_rtv_off != -1)
				{
					if (curr_plt == PipelineGraphics)
						d3d12_command_list->SetGraphicsRootDescriptorTable(id + 0, dss[i]->d3d12_srv_gpu_handle);
					else
						d3d12_command_list->SetComputeRootDescriptorTable(id + 0, dss[i]->d3d12_srv_gpu_handle);
				}
				if (dss[i]->d3d12_sp_off != -1)
				{
					if (curr_plt == PipelineGraphics)
						d3d12_command_list->SetGraphicsRootDescriptorTable(id + 1, dss[i]->d3d12_sp_gpu_handle);
					else
						d3d12_command_list->SetComputeRootDescriptorTable(id + 1, dss[i]->d3d12_sp_gpu_handle);
				}
			}
#elif USE_VULKAN
			std::vector<VkDescriptorSet> vk_sets(dss.size());
			for (auto i = 0; i < dss.size(); i++)
				vk_sets[i++] = dss[i]->vk_descriptor_set;
			vkCmdBindDescriptorSets(vk_command_buffer, to_vk(curr_plt), curr_pll->vk_pipeline_layout, idx, vk_sets.size(), vk_sets.data(), 0, nullptr);
#endif
		}

		void CommandBufferPrivate::bind_vertex_buffer(BufferPtr buf, uint id, uint stride)
		{
#if USE_D3D12
			D3D12_VERTEX_BUFFER_VIEW view;
			view.BufferLocation = buf->d3d12_resource->GetGPUVirtualAddress();
			view.StrideInBytes = stride;
			view.SizeInBytes = buf->size;
			d3d12_command_list->IASetVertexBuffers(0, 1, &view);
#elif USE_VULKAN
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(vk_command_buffer, id, 1, &buf->vk_buffer, &offset);
#endif
		}

		void CommandBufferPrivate::bind_index_buffer(BufferPtr buf, IndiceType t)
		{
#if USE_D3D12
			D3D12_INDEX_BUFFER_VIEW view;
			view.BufferLocation = buf->d3d12_resource->GetGPUVirtualAddress();
			view.Format = t == IndiceTypeUint ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			view.SizeInBytes = buf->size;
			d3d12_command_list->IASetIndexBuffer(&view);
#elif USE_VULKAN
			vkCmdBindIndexBuffer(vk_command_buffer, buf->vk_buffer, 0, t == IndiceTypeUint ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16);
#endif
		}

		void CommandBufferPrivate::push_constant(uint offset, uint size, const void* data)
		{
#if USE_D3D12
			auto idx = curr_pll->dsls.size() * 2;
			if (curr_plt == PipelineGraphics)
				d3d12_command_list->SetGraphicsRoot32BitConstants(idx, size / 4, data, offset / 4);
			else
				d3d12_command_list->SetComputeRoot32BitConstants(idx, size / 4, data, offset / 4);
#elif USE_VULKAN
			vkCmdPushConstants(vk_command_buffer, curr_pll->vk_pipeline_layout, to_vk_flags<ShaderStageFlags>(ShaderStageAll), offset, size, data);
#endif
		}

		void CommandBufferPrivate::draw(uint count, uint instance_count, uint first_vertex, uint first_instance)
		{
#if USE_D3D12
			d3d12_command_list->DrawInstanced(count, instance_count, first_vertex, first_instance);
#elif USE_VULKAN
			vkCmdDraw(vk_command_buffer, count, instance_count, first_vertex, first_instance);
#endif
		}

		void CommandBufferPrivate::draw_indexed(uint count, uint first_index, int vertex_offset, uint instance_count, uint first_instance)
		{
#if USE_D3D12
			d3d12_command_list->DrawIndexedInstanced(count, instance_count, first_index, vertex_offset, first_instance);
#elif USE_VULKAN
			vkCmdDrawIndexed(vk_command_buffer, count, instance_count, first_index, vertex_offset, first_instance);
#endif
		}

		void CommandBufferPrivate::draw_indirect(BufferPtr buf, uint offset, uint count)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdDrawIndirect(vk_command_buffer, buf->vk_buffer, offset * sizeof(VkDrawIndirectCommand), count, sizeof(VkDrawIndirectCommand));
#endif
		}

		void CommandBufferPrivate::draw_indexed_indirect(BufferPtr buf, uint offset, uint count)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdDrawIndexedIndirect(vk_command_buffer, buf->vk_buffer, offset * sizeof(VkDrawIndexedIndirectCommand), count, sizeof(VkDrawIndexedIndirectCommand));
#endif
		}

		void CommandBufferPrivate::dispatch(const uvec3& v)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdDispatch(vk_command_buffer, v.x, v.y, v.z);
#endif
		}

		void CommandBufferPrivate::draw_mesh_tasks(const uvec3& v)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdDrawMeshTasks(vk_command_buffer, v.x, v.y, v.z);
#endif
		}

		void CommandBufferPrivate::buffer_barrier(BufferPtr buf, AccessFlags src_access, AccessFlags dst_access, PipelineStageFlags src_stage, PipelineStageFlags dst_stage)
		{
#if USE_D3D12
			auto old_state = to_dx(src_access);
			auto new_state = to_dx(dst_access);
			if (old_state != new_state)
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = buf->d3d12_resource;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = old_state;
				barrier.Transition.StateAfter = new_state;
				d3d12_command_list->ResourceBarrier(1, &barrier);
			}
#elif USE_VULKAN
			VkBufferMemoryBarrier barrier;
			barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
			barrier.pNext = nullptr;
			barrier.srcAccessMask = to_vk_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_vk_flags<AccessFlags>(dst_access);
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

			vkCmdPipelineBarrier(vk_command_buffer, to_vk_flags<PipelineStageFlags>(src_stage), to_vk_flags<PipelineStageFlags>(dst_stage),
				0, 0, nullptr, 1, &barrier, 0, nullptr);
#endif
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

#if USE_D3D12
			auto old_state = to_dx(old_layout, img->format);
			auto new_state = to_dx(new_layout, img->format);
			if (new_state != old_state)
			{
				D3D12_RESOURCE_BARRIER barrier;
				barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				barrier.Transition.pResource = img->d3d12_resource;
				barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				barrier.Transition.StateBefore = old_state;
				barrier.Transition.StateAfter = new_state;
				d3d12_command_list->ResourceBarrier(1, &barrier);
			}
#elif USE_VULKAN

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
			barrier.srcAccessMask = to_vk_flags<AccessFlags>(src_access);
			barrier.dstAccessMask = to_vk_flags<AccessFlags>(dst_access);
			barrier.oldLayout = to_vk(old_layout, img->format);
			barrier.newLayout = to_vk(new_layout, img->format);
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = img->vk_image;
			barrier.subresourceRange.aspectMask = to_vk_flags<ImageAspectFlags>(aspect_from_format(img->format));
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

			vkCmdPipelineBarrier(vk_command_buffer, to_vk_flags<PipelineStageFlags>(src_stage), to_vk_flags<PipelineStageFlags>(dst_stage),
				0, 0, nullptr, 0, nullptr, 1, &barrier);
#endif
		}

#if USE_D3D12

#elif USE_VULKAN
		VkBufferCopy to_vk(const BufferCopy& src)
		{
			VkBufferCopy ret = {};
			ret.srcOffset = src.src_off;
			ret.dstOffset = src.dst_off;
			ret.size = src.size;
			return ret;
		}

		VkImageCopy to_vk(const ImageCopy& src, VkImageAspectFlags aspect)
		{
			VkImageCopy ret = {};
			ret.srcSubresource.aspectMask = aspect;
			ret.srcSubresource.mipLevel = src.src_sub.base_level;
			ret.srcSubresource.baseArrayLayer = src.src_sub.base_layer;
			ret.srcSubresource.layerCount = src.src_sub.layer_count;
			ret.srcOffset.x = src.src_off.x;
			ret.srcOffset.y = src.src_off.y;
			ret.srcOffset.z = src.src_off.z;
			ret.dstSubresource.aspectMask = aspect;
			ret.dstSubresource.mipLevel = src.dst_sub.base_level;
			ret.dstSubresource.baseArrayLayer = src.dst_sub.base_layer;
			ret.dstSubresource.layerCount = src.dst_sub.layer_count;
			ret.dstOffset.x = src.dst_off.x;
			ret.dstOffset.y = src.dst_off.y;
			ret.dstOffset.z = 0;
			ret.extent.width = src.ext.x;
			ret.extent.height = src.ext.y;
			ret.extent.depth = src.ext.z;
			return ret;
		}

		VkBufferImageCopy to_vk(const BufferImageCopy& src, VkImageAspectFlags aspect)
		{
			VkBufferImageCopy ret = {};
			ret.bufferOffset = src.buf_off;
			ret.imageOffset.x = src.img_off.x;
			ret.imageOffset.y = src.img_off.y;
			ret.imageOffset.z = src.img_off.z;
			ret.imageExtent.width = src.img_ext.x;
			ret.imageExtent.height = src.img_ext.y;
			ret.imageExtent.depth = src.img_ext.z;
			ret.imageSubresource.aspectMask = aspect;
			ret.imageSubresource.mipLevel = src.img_sub.base_level;
			ret.imageSubresource.baseArrayLayer = src.img_sub.base_layer;
			ret.imageSubresource.layerCount = src.img_sub.layer_count;
			return ret;
		}

		VkImageBlit to_vk(const ImageBlit& src)
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
#endif

		void CommandBufferPrivate::copy_buffer(BufferPtr src, BufferPtr dst, std::span<BufferCopy> copies)
		{
#if USE_D3D12
			for (auto& cpy : copies)
				d3d12_command_list->CopyBufferRegion(dst->d3d12_resource, cpy.dst_off, src->d3d12_resource, cpy.src_off, cpy.size);
#elif USE_VULKAN
			std::vector<VkBufferCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_vk(copies[i]);
			vkCmdCopyBuffer(vk_command_buffer, src->vk_buffer, dst->vk_buffer, vk_copies.size(), vk_copies.data());
#endif
		}

		void CommandBufferPrivate::copy_image(ImagePtr src, ImagePtr dst, std::span<ImageCopy> copies)
		{
#if USE_D3D12

#elif USE_VULKAN
			auto aspect = to_vk_flags<ImageAspectFlags>(aspect_from_format(src->format));

			std::vector<VkImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_vk(copies[i], aspect);
			vkCmdCopyImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
#endif
		}

		void CommandBufferPrivate::copy_buffer_to_image(BufferPtr src, ImagePtr dst, std::span<BufferImageCopy> copies)
		{
#if USE_D3D12
			for (auto& cpy : copies)
			{
				auto sub_idx = (cpy.img_sub.base_layer * dst->n_levels) + cpy.img_sub.base_level;
				D3D12_TEXTURE_COPY_LOCATION src_location;
				src_location.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
				src_location.pResource = src->d3d12_resource;
				src_location.PlacedFootprint.Offset = cpy.buf_off;
				src_location.PlacedFootprint.Footprint.Format = to_dx(dst->format);
				src_location.PlacedFootprint.Footprint.Width = cpy.img_ext.x;
				src_location.PlacedFootprint.Footprint.Height = cpy.img_ext.y;
				src_location.PlacedFootprint.Footprint.Depth = cpy.img_ext.z;
				src_location.PlacedFootprint.Footprint.RowPitch = image_pitch(dst->pixel_size * cpy.img_ext.x, 256);
				D3D12_TEXTURE_COPY_LOCATION dst_location;
				dst_location.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
				dst_location.pResource = dst->d3d12_resource;
				dst_location.SubresourceIndex = sub_idx;
				D3D12_BOX box;
				box.left = 0;
				box.top = 0;
				box.front = 0;
				box.right = cpy.img_ext.x;
				box.bottom = cpy.img_ext.y;
				box.back = cpy.img_ext.z;
				d3d12_command_list->CopyTextureRegion(&dst_location, cpy.img_off.x, cpy.img_off.y, cpy.img_off.z, &src_location, &box);
			}
#elif USE_VULKAN
			auto aspect = to_vk_flags<ImageAspectFlags>(aspect_from_format(dst->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_vk(copies[i], aspect);
			vkCmdCopyBufferToImage(vk_command_buffer, src->vk_buffer, dst->vk_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, vk_copies.size(), vk_copies.data());
#endif
		}

		void CommandBufferPrivate::copy_image_to_buffer(ImagePtr src, BufferPtr dst, std::span<BufferImageCopy> copies)
		{
#if USE_D3D12

#elif USE_VULKAN
			auto aspect = to_vk_flags<ImageAspectFlags>(aspect_from_format(src->format));

			std::vector<VkBufferImageCopy> vk_copies(copies.size());
			for (auto i = 0; i < vk_copies.size(); i++)
				vk_copies[i] = to_vk(copies[i], aspect);
			vkCmdCopyImageToBuffer(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->vk_buffer, vk_copies.size(), vk_copies.data());
#endif
		}

		void CommandBufferPrivate::blit_image(ImagePtr src, ImagePtr dst, std::span<ImageBlit> blits, Filter filter)
		{
#if USE_D3D12

#elif USE_VULKAN
			std::vector<VkImageBlit> vk_blits(blits.size());
			for (auto i = 0; i < vk_blits.size(); i++)
				vk_blits[i] = to_vk(blits[i]);
			vkCmdBlitImage(vk_command_buffer, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src->vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				vk_blits.size(), vk_blits.data(), to_vk(filter));
#endif
		}

		void CommandBufferPrivate::clear_color_image(ImagePtr img, const ImageSub& sub, const vec4& color)
		{
#if USE_D3D12

#elif USE_VULKAN
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
#endif
		}

		void CommandBufferPrivate::clear_depth_image(ImagePtr img, const ImageSub& sub, float depth)
		{
#if USE_D3D12

#elif USE_VULKAN
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
#endif
		}

		void CommandBufferPrivate::set_event(EventPtr ev)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdSetEvent(vk_command_buffer, ev->vk_event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
#endif
		}

		void CommandBufferPrivate::reset_event(EventPtr ev)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdResetEvent(vk_command_buffer, ev->vk_event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
#endif
		}

		void CommandBufferPrivate::wait_event(EventPtr ev)
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdWaitEvents(vk_command_buffer, 1, &ev->vk_event, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0, nullptr, 0, nullptr, 0, nullptr);
#endif
		}

		void CommandBufferPrivate::begin_debug_label(const std::string& str)
		{
#if USE_D3D12

#elif USE_VULKAN
			VkDebugUtilsLabelEXT info = {};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
			info.pLabelName = str.c_str();
			vkCmdBeginDebugUtilsLabel(vk_command_buffer, &info);
#endif
		}

		void CommandBufferPrivate::end_debug_label()
		{
#if USE_D3D12

#elif USE_VULKAN
			vkCmdEndDebugUtilsLabel(vk_command_buffer);
#endif
		}

		void CommandBufferPrivate::end()
		{
#if USE_D3D12
			check_dx_result(d3d12_command_list->Close());
#elif USE_VULKAN
			if (want_executed_time)
				vkCmdWriteTimestamp(vk_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vk_query_pool, 1);

			check_vk_result(vkEndCommandBuffer(vk_command_buffer));
#endif
		}

		void CommandBufferPrivate::calc_executed_time()
		{
#if USE_D3D12

#elif USE_VULKAN
			if (vk_query_pool)
			{
				uint64 timestamps[2];
				vkGetQueryPoolResults(device->vk_device, vk_query_pool, 0, 2, sizeof(uint64) * 2, timestamps, sizeof(uint64), VK_QUERY_RESULT_64_BIT);
				last_executed_time = timestamps[1] - timestamps[0];
			}
#endif
		}

		struct CommandBufferCreate : CommandBuffer::Create
		{
			CommandBufferPtr operator()(CommandPoolPtr pool, bool sub) override
			{
				if (!pool)
					pool = CommandPool::get();

				auto ret = new CommandBufferPrivate;
				ret->pool = pool;

#if USE_D3D12
				check_dx_result(device->d3d12_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pool->d3d12_command_allocator, nullptr, IID_PPV_ARGS(&ret->d3d12_command_list)));
				check_dx_result(ret->d3d12_command_list->Close());
				register_object(ret->d3d12_command_list, "Command Buffer", ret);
#elif USE_VULKAN
				VkCommandBufferAllocateInfo info;
				info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				info.pNext = nullptr;
				info.level = !sub ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
				info.commandPool = pool->vk_command_pool;
				info.commandBufferCount = 1;

				check_vk_result(vkAllocateCommandBuffers(device->vk_device, &info, &ret->vk_command_buffer));
				register_object(ret->vk_command_buffer, "Command Buffer", ret);

				ret->begin();
				ret->end();
#endif

				return ret;
			}
		}CommandBuffer_create;
		CommandBuffer::Create& CommandBuffer::create = CommandBuffer_create;

		QueuePrivate::~QueuePrivate()
		{
			if (app_exiting) return;

			d3d12_queue->Release();
		}

		void QueuePrivate::wait_idle()
		{
#if USE_D3D12
			idle_fence->value++;
			d3d12_queue->Signal(idle_fence->d3d12_fence, idle_fence->value);
			idle_fence->wait();
#elif USE_VULKAN
			check_vk_result(vkQueueWaitIdle(vk_queue));
#endif
		}

		void QueuePrivate::submit(std::span<CommandBufferPtr> commandbuffers, std::span<SemaphorePtr> wait_semaphores, std::span<SemaphorePtr> signal_semaphores, FencePtr signal_fence)
		{
#ifdef USE_D3D12
			std::vector<ID3D12CommandList*> list;
			list.resize(commandbuffers.size());
			for (auto i = 0; i < commandbuffers.size(); i++)
				list[i] = commandbuffers[i]->d3d12_command_list;
			d3d12_queue->ExecuteCommandLists(list.size(), list.data());
#elif USE_VULKAN
			std::vector<VkCommandBuffer> vk_cbs;
			vk_cbs.resize(commandbuffers.size());
			for (auto i = 0; i < vk_cbs.size(); i++)
				vk_cbs[i] = commandbuffers[i]->vk_command_buffer;

			std::vector<VkSemaphore> vk_wait_smps;
			vk_wait_smps.resize(wait_semaphores.size());
			for (auto i = 0; i < vk_wait_smps.size(); i++)
				vk_wait_smps[i] = wait_semaphores[i]->vk_semaphore;
			std::vector<VkPipelineStageFlags> vk_wait_stages;
			vk_wait_stages.resize(wait_semaphores.size());
			for (auto i = 0; i < vk_wait_stages.size(); i++)
				vk_wait_stages[i] = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

			std::vector<VkSemaphore> vk_signal_smps;
			vk_signal_smps.resize(signal_semaphores.size());
			for (auto i = 0; i < vk_signal_smps.size(); i++)
				vk_signal_smps[i] = signal_semaphores[i]->vk_semaphore;

			VkSubmitInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.waitSemaphoreCount = vk_wait_smps.size();
			info.pWaitSemaphores = vk_wait_smps.data();
			info.pWaitDstStageMask = vk_wait_stages.data();
			info.commandBufferCount = commandbuffers.size();
			info.pCommandBuffers = vk_cbs.data();
			info.signalSemaphoreCount = vk_signal_smps.size();
			info.pSignalSemaphores = vk_signal_smps.data();

			check_vk_result(vkQueueSubmit(vk_queue, 1, &info, signal_fence ? signal_fence->vk_fence : nullptr));
#endif

#ifdef USE_D3D12
			if (signal_fence)
			{
				signal_fence->value++;
				d3d12_queue->Signal(signal_fence->d3d12_fence, signal_fence->value);
			}
#endif
		}

		void QueuePrivate::present(std::span<SwapchainPtr> swapchains, std::span<SemaphorePtr> wait_semaphores)
		{
#ifdef USE_D3D12
			printf("you should present using submit_and_present!!\n");
			assert(0);
#elif USE_VULKAN
			std::vector<VkSemaphore> vk_wait_smps;
			vk_wait_smps.resize(wait_semaphores.size());
			for (auto i = 0; i < vk_wait_smps.size(); i++)
				vk_wait_smps[i] = wait_semaphores[i]->vk_semaphore;

			std::vector<VkSwapchainKHR> vk_scs;
			std::vector<uint> indices;
			vk_scs.resize(swapchains.size());
			indices.resize(swapchains.size());
			for (auto i = 0; i < vk_scs.size(); i++)
			{
				vk_scs[i] = swapchains[i]->vk_swapchain;
				indices[i] = swapchains[i]->image_index;
			}

			VkPresentInfoKHR info = {};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.waitSemaphoreCount = vk_wait_smps.size();
			info.pWaitSemaphores = vk_wait_smps.data();
			info.swapchainCount = vk_scs.size();
			info.pSwapchains = vk_scs.data();
			info.pImageIndices = indices.data();
			check_vk_result(vkQueuePresentKHR(vk_queue, &info));
#endif
		}

		void QueuePrivate::submit_and_present(std::span<CommandBufferPtr> commandbuffers, std::span<SemaphorePtr> wait_semaphores, SemaphorePtr signal_semaphore, FencePtr signal_fence, std::span<SwapchainPtr> swapchains)
		{
#ifdef USE_D3D12
			std::vector<ID3D12CommandList*> list;
			list.resize(commandbuffers.size());
			for (auto i = 0; i < commandbuffers.size(); i++)
				list[i] = commandbuffers[i]->d3d12_command_list;
			d3d12_queue->ExecuteCommandLists(list.size(), list.data());

			for (auto i = 0; i < swapchains.size(); i++)
				swapchains[i]->d3d12_swapchain->Present(1, 0);

			signal_fence->value++;
			d3d12_queue->Signal(signal_fence->d3d12_fence, signal_fence->value);

#elif USE_VULKAN
			submit(commandbuffers, wait_semaphores, { &signal_semaphore, 1 }, signal_fence);
			present(swapchains, wait_semaphores);
#endif
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

#if USE_D3D12
				D3D12_COMMAND_QUEUE_DESC desc = {};
				desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
				desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

				check_dx_result(device->d3d12_device->CreateCommandQueue(&desc, IID_PPV_ARGS(&ret->d3d12_queue)));
				ret->idle_fence = Fence::create();
#elif USE_VULKAN
				vkGetDeviceQueue(device->vk_device, queue_family_idx, 0, &ret->vk_queue);
#endif

				return ret;
			}
		}Queue_create;
		Queue::Create& Queue::create = Queue_create;

		SemaphorePrivate::~SemaphorePrivate()
		{
			if (app_exiting) return;

#if USE_VULKAN
			vkDestroySemaphore(device->vk_device, vk_semaphore, nullptr);
			unregister_object(vk_semaphore);
#endif
		}

		struct SemaphoreCreate : Semaphore::Create
		{
			SemaphorePtr operator()() override
			{
				auto ret = new SemaphorePrivate;

#if USE_VULKAN
				VkSemaphoreCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				check_vk_result(vkCreateSemaphore(device->vk_device, &info, nullptr, &ret->vk_semaphore));
				register_object(ret->vk_semaphore, "Semaphore", ret);
#endif

				return ret;
			}
		}Semaphore_create;
		Semaphore::Create& Semaphore::create = Semaphore_create;

		EventPrivate::~EventPrivate()
		{
			if (app_exiting) return;

#if USE_VULKAN
			vkDestroyEvent(device->vk_device, vk_event, nullptr);
			unregister_object(vk_event);
#endif
		}

		struct EventCreate : Event::Create
		{
			EventPtr operator()() override
			{
				auto ret = new EventPrivate;

#if USE_VULKAN
				VkEventCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
				check_vk_result(vkCreateEvent(device->vk_device, &info, nullptr, &ret->vk_event));
				register_object(ret->vk_event, "Event", ret);
#endif

				return ret;
			}
		}Event_create;
		Event::Create& Event::create = Event_create;

		FencePrivate::~FencePrivate()
		{
			if (app_exiting) return;

#if USE_D3D12
			d3d12_fence->Release();
			unregister_object(d3d12_fence);
#elif USE_VULKAN
			vkDestroyFence(device->vk_device, vk_fence, nullptr);
			unregister_object(vk_fence);
#endif
		}

		void FencePrivate::wait(bool auto_reset)
		{
#ifdef USE_D3D12
			if (d3d12_fence->GetCompletedValue() != value)
			{
				check_dx_result(d3d12_fence->SetEventOnCompletion(value, d3d12_event));
				WaitForSingleObject(d3d12_event, 0xffffffff);
			}
#elif USE_VULKAN
			if (value > 0)
			{
				check_vk_result(vkWaitForFences(device->vk_device, 1, &vk_fence, true, UINT64_MAX));
				if (auto_reset)
				{
					check_vk_result(vkResetFences(device->vk_device, 1, &vk_fence));
					value = 0;
				}
			}
#endif
		}

		struct FenceCreate : Fence::Create
		{
			FencePtr operator()(bool signaled) override
			{
				auto ret = new FencePrivate;

#if USE_D3D12
				check_dx_result(device->d3d12_device->CreateFence(ret->value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&ret->d3d12_fence)));
				ret->d3d12_event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
				register_object(ret->d3d12_fence, "Fence", ret);
#elif USE_VULKAN
				VkFenceCreateInfo info = {};
				info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				if (signaled)
				{
					info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
					ret->value = 1;
				}
				check_vk_result(vkCreateFence(device->vk_device, &info, nullptr, &ret->vk_fence));
				register_object(ret->vk_fence, "Fence", ret);
#endif

				return ret;
			}
		}Fence_create;
		Fence::Create& Fence::create = Fence_create;
	}
}
