// MIT License
// 
// Copyright (c) 2019 wjs
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
#include "renderpass_private.h"

#include <memory>
#include <algorithm>

namespace flame
{
	namespace graphics
	{
		inline bool operator==(const AttachmentInfo &lhs, const AttachmentInfo &rhs)
		{
			return lhs.format == rhs.format && lhs.sample_count == rhs.sample_count && lhs.clear == rhs.clear;
		}

		inline bool operator==(const SubpassInfo &lhs, const SubpassInfo &rhs)
		{
			return lhs.color_attachments == rhs.color_attachments && lhs.resolve_attachments == rhs.resolve_attachments && lhs.depth_attachment == rhs.depth_attachment;
		}

		inline bool operator==(const DependencyInfo &lhs, const DependencyInfo &rhs)
		{
			return lhs.src_subpass == rhs.src_subpass && lhs.dst_subpass == rhs.dst_subpass;
		}

		inline RenderpassPrivate::RenderpassPrivate(Device *_d, const RenderpassInfo &_info)
		{
			d = (DevicePrivate*)_d;

			info = _info;

#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(info.attachments.size());
			for (auto i = 0; i < vk_attachments.size(); i++)
			{
				vk_attachments[i].flags = 0;
				vk_attachments[i].format = Z(info.attachments[i].format);
				vk_attachments[i].samples = Z(info.attachments[i].sample_count);
				vk_attachments[i].loadOp = info.attachments[i].clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				vk_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				auto fmt = info.attachments[i].format;
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				{
					if (fmt >= Format_Swapchain_Begin && fmt <= Format_Swapchain_End)
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					else
						vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				else
					vk_attachments[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(info.subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_resolve_refs(info.subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(info.subpasses.size());
			std::vector<VkSubpassDescription> vk_subpasses(info.subpasses.size());
			for (auto i = 0; i < vk_subpasses.size(); i++)
			{
				vk_subpasses[i].flags = 0;
				vk_subpasses[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				vk_subpasses[i].inputAttachmentCount = 0;
				vk_subpasses[i].pInputAttachments = nullptr;
				vk_subpasses[i].colorAttachmentCount = 0;
				vk_subpasses[i].pColorAttachments = nullptr;
				vk_subpasses[i].pResolveAttachments = nullptr;
				vk_subpasses[i].pDepthStencilAttachment = nullptr;
				vk_subpasses[i].preserveAttachmentCount = 0;
				vk_subpasses[i].pPreserveAttachments = nullptr;
				if (info.subpasses[i].color_attachments.size() > 0)
				{
					vk_color_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[info.subpasses[i].color_attachments.size()]);
					for (auto j = 0; j < info.subpasses[i].color_attachments.size(); j++)
					{
						vk_color_refs[i][j].attachment = info.subpasses[i].color_attachments[j];
						vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].colorAttachmentCount = info.subpasses[i].color_attachments.size();
					vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
				}
				if (!info.subpasses[i].resolve_attachments.empty())
				{
					vk_resolve_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[info.subpasses[i].color_attachments.size()]);
					for (auto j = 0; j < info.subpasses[i].color_attachments.size(); j++)
					{
						vk_resolve_refs[i][j].attachment = info.subpasses[i].resolve_attachments[j];
						vk_resolve_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].pResolveAttachments = vk_resolve_refs[i].get();
				}
				if (info.subpasses[i].depth_attachment != -1)
				{
					vk_depth_refs[i] = std::unique_ptr<VkAttachmentReference>(new VkAttachmentReference);
					vk_depth_refs[i]->attachment = info.subpasses[i].depth_attachment;
					vk_depth_refs[i]->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					vk_subpasses[i].pDepthStencilAttachment = vk_depth_refs[i].get();
				}
			}

			std::vector<VkSubpassDependency> vk_dependencies(info.dependencies.size());
			for (auto i = 0; i < vk_dependencies.size(); i++)
			{
				vk_dependencies[i].srcSubpass = info.dependencies[i].src_subpass;
				vk_dependencies[i].dstSubpass = info.dependencies[i].dst_subpass;
				vk_dependencies[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				vk_dependencies[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				vk_dependencies[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				vk_dependencies[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				vk_dependencies[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			VkRenderPassCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			create_info.attachmentCount = vk_attachments.size();
			create_info.pAttachments = vk_attachments.data();
			create_info.subpassCount = vk_subpasses.size();
			create_info.pSubpasses = vk_subpasses.data();
			create_info.dependencyCount = vk_dependencies.size();
			create_info.pDependencies = vk_dependencies.data();

			vk_chk_res(vkCreateRenderPass(d->v, &create_info, nullptr, &v));
#endif
		}

		inline RenderpassPrivate::~RenderpassPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyRenderPass(d->v, v, nullptr);
#endif
		}

		int Renderpass::attachment_count() const
		{
			return ((RenderpassPrivate*)this)->info.attachments.size();
		}

		std::vector<RenderpassPrivate*> created_renderpasses;

		Renderpass *Renderpass::get(Device *d, const RenderpassInfo &info)
		{
			for (auto &r : created_renderpasses)
			{
				if (r->info.attachments == info.attachments && r->info.subpasses == info.subpasses && r->info.dependencies == info.dependencies)
				{
					r->ref_count++;
					return r;
				}
			}

			auto r = new RenderpassPrivate(d, info);
			r->ref_count = 1;
			created_renderpasses.push_back(r);
			return r;
		}

		void Renderpass::release(Renderpass *r)
		{
			if (((RenderpassPrivate*)r)->ref_count == 1)
			{
				for (auto it = created_renderpasses.begin(); it != created_renderpasses.end(); it++)
				{
					if ((*it) == r)
					{
						created_renderpasses.erase(it);
						break;
					}
				}
				delete (RenderpassPrivate*)r;
			}
			else
				((RenderpassPrivate*)r)->ref_count--;
		}

		inline ClearvaluesPrivate::ClearvaluesPrivate(Renderpass *r)
		{
			for (auto i = 0; i < ((RenderpassPrivate*)r)->info.attachments.size(); i++)
			{
				auto fmt = ((RenderpassPrivate*)r)->info.attachments[i].format;
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
#if defined(FLAME_VULKAN)
					v.push_back({});
#elif defined(FLAME_D3D12)
					v.push_back(Vec4(0.f));
#endif
				else
#if defined(FLAME_VULKAN)
					v.push_back({ 1, 0.f });
#elif defined(FLAME_D3D12)
					v.push_back(Vec4(1.f, 0.f, 0.f, 0.f));
#endif
			}
		}

		inline ClearvaluesPrivate::~ClearvaluesPrivate()
		{
		}

		inline void ClearvaluesPrivate::set(int idx, const Bvec4 &col)
		{
#if defined(FLAME_VULKAN)
			v[idx].color.float32[0] = col.x / 255.f;
			v[idx].color.float32[1] = col.y / 255.f;
			v[idx].color.float32[2] = col.z / 255.f;
			v[idx].color.float32[3] = col.w / 255.f;
#elif defined(FLAME_D3D12)
			v[idx].x = col.x / 255.0;
			v[idx].y = col.y / 255.0;
			v[idx].z = col.z / 255.0;
			v[idx].w = col.w / 255.0;
#endif
		}

		Renderpass* ClearValues::renderpass() const
		{
			return ((ClearvaluesPrivate*)this)->renderpass;
		}

		void ClearValues::set(int idx, const Bvec4 &col)
		{
			((ClearvaluesPrivate*)this)->set(idx, col);
		}

		ClearValues *ClearValues::create(Renderpass *f)
		{
			return new ClearvaluesPrivate(f);
		}

		void ClearValues::destroy(ClearValues *c)
		{
			delete (ClearvaluesPrivate*)c;
		}
	}
}

