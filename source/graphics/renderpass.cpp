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

namespace flame
{
	namespace graphics
	{
		struct AttachmentInfo$
		{
			Format$ format$i;
			bool clear$i;
			SampleCount$ sample_count$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS AttachmentInfo$() :
				format$i(Format_R8G8B8A8_UNORM),
				clear$i(true),
				sample_count$i(SampleCount_1)
			{
			}

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				auto info = new AttachmentInfo;
				info->format = format$i;
				info->clear = clear$i;
				info->sample_count = sample_count$i;
				out$o = info;
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				delete (AttachmentInfo*)out$o;
			}

		}bp_attachmentinfo_unused;

		struct SubpassInfo$
		{
			Array<uint> color_attachments$i;
			Array<uint> resolve_attachments$i;
			int depth_attachment$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS SubpassInfo$() :
				depth_attachment$i(-1)
			{
			}

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				auto info = new SubpassInfo;
				info->color_attachments = color_attachments$i;
				info->resolve_attachments = resolve_attachments$i;
				info->depth_attachment = depth_attachment$i;
				out$o = info;
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				delete (SubpassInfo*)out$o;
			}

		}bp_subpassinfo_unused;

		struct Renderpass$
		{
			void* device$i;
			Array<void*> attachments$i;
			Array<void*> subpasses$i;
			Array<Vec<2, uint>> dependencies$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				if (device$i)
				{
					RenderpassInfo info;
					info.attachments = attachments$i;
					info.subpasses = subpasses$i;
					info.dependencies = dependencies$i;
					out$o = Renderpass::create((Device*)device$i, info);
				}
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				delete (SubpassInfo*)out$o;
			}

		}bp_renderpass_unused;

		RenderpassPrivate::RenderpassPrivate(Device *_d, const RenderpassInfo& info)
		{
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(info.attachments.size);
			for (auto i = 0; i < vk_attachments.size(); i++)
			{
				auto at_info = (AttachmentInfo*)info.attachments.v[i];

				vk_attachments[i].flags = 0;
				vk_attachments[i].format = Z(at_info->format);
				vk_attachments[i].samples = Z(at_info->sample_count);
				vk_attachments[i].loadOp = at_info->clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				vk_attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				vk_attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				vk_attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				auto fmt = at_info->format;
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

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(info.subpasses.size);
			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_resolve_refs(info.subpasses.size);
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(info.subpasses.size);
			std::vector<VkSubpassDescription> vk_subpasses(info.subpasses.size);
			for (auto i = 0; i < vk_subpasses.size(); i++)
			{
				auto sp_info = (SubpassInfo*)info.subpasses.v[i];

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
				if (sp_info->color_attachments.size > 0)
				{
					vk_color_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[sp_info->color_attachments.size]);
					for (auto j = 0; j < sp_info->color_attachments.size; j++)
					{
						vk_color_refs[i][j].attachment = sp_info->color_attachments.v[j];
						vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].colorAttachmentCount = sp_info->color_attachments.size;
					vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
				}
				if (sp_info->resolve_attachments.size != 0)
				{
					vk_resolve_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[sp_info->color_attachments.size]);
					for (auto j = 0; j < sp_info->color_attachments.size; j++)
					{
						vk_resolve_refs[i][j].attachment = sp_info->resolve_attachments.v[j];
						vk_resolve_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].pResolveAttachments = vk_resolve_refs[i].get();
				}
				if (sp_info->depth_attachment != -1)
				{
					vk_depth_refs[i] = std::unique_ptr<VkAttachmentReference>(new VkAttachmentReference);
					vk_depth_refs[i]->attachment = sp_info->depth_attachment;
					vk_depth_refs[i]->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					vk_subpasses[i].pDepthStencilAttachment = vk_depth_refs[i].get();
				}
			}

			std::vector<VkSubpassDependency> vk_dependencies(info.dependencies.size);
			for (auto i = 0; i < info.dependencies.size; i++)
			{
				auto& dp_info = info.dependencies.v[i];

				vk_dependencies[i].srcSubpass = dp_info.x();
				vk_dependencies[i].dstSubpass = dp_info.y();
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

			attachments.resize(info.attachments.size);
			for (auto i = 0; i < info.attachments.size; i++)
				attachments[i] = ((AttachmentInfo*)(info.attachments.v[i]))->format;
#endif
		}

		RenderpassPrivate::~RenderpassPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyRenderPass(d->v, v, nullptr);
#endif
		}

		int Renderpass::attachment_count() const
		{
			return ((RenderpassPrivate*)this)->attachments.size();
		}

		Renderpass* Renderpass::create(Device *d, const RenderpassInfo& info)
		{
			return new RenderpassPrivate(d, info);
		}

		void Renderpass::destroy(Renderpass *r)
		{
			delete (RenderpassPrivate*)r;
		}

		ClearvaluesPrivate::ClearvaluesPrivate(Renderpass *r)
		{
			renderpass = (RenderpassPrivate*)r;

			for (auto i = 0; i < renderpass->attachments.size(); i++)
			{
				auto fmt = ((RenderpassPrivate*)r)->attachments[i];
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
#if defined(FLAME_VULKAN)
					v.push_back({});
#elif defined(FLAME_D3D12)
					v.push_back(Vec4f(0.f));
#endif
				else
#if defined(FLAME_VULKAN)
					v.push_back({ 1, 0.f });
#elif defined(FLAME_D3D12)
					v.push_back(Vec4f(1.f, 0.f, 0.f, 0.f));
#endif
			}
		}

		ClearvaluesPrivate::~ClearvaluesPrivate()
		{
		}

		void ClearvaluesPrivate::set(int idx, const Vec4c& col)
		{
#if defined(FLAME_VULKAN)
			v[idx].color.float32[0] = col.x() / 255.f;
			v[idx].color.float32[1] = col.y() / 255.f;
			v[idx].color.float32[2] = col.z() / 255.f;
			v[idx].color.float32[3] = col.w() / 255.f;
#elif defined(FLAME_D3D12)
			v[idx].x() = col.x() / 255.0;
			v[idx].y() = col.y() / 255.0;
			v[idx].z() = col.z() / 255.0;
			v[idx].w() = col.w() / 255.0;
#endif
		}

		Renderpass* Clearvalues::renderpass() const
		{
			return ((ClearvaluesPrivate*)this)->renderpass;
		}

		void Clearvalues::set(int idx, const Vec4c& col)
		{
			((ClearvaluesPrivate*)this)->set(idx, col);
		}

		Clearvalues* Clearvalues::create(Renderpass *f)
		{
			return new ClearvaluesPrivate(f);
		}

		void Clearvalues::destroy(Clearvalues*c)
		{
			delete (ClearvaluesPrivate*)c;
		}

		struct Clearvalues$
		{
			void* renderpass$i;
			Array<Vec4c> colors$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				if (renderpass$i)
					out$o = Clearvalues::create((Renderpass*)renderpass$i);
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				Clearvalues::destroy((Clearvalues*)out$o);
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (out$o)
				{
					auto cv = (Clearvalues*)out$o;
					auto count = cv->renderpass()->attachment_count();
					for (auto i = 0; i < count; i++)
						cv->set(i, colors$i.v[i]);
				}
			}

		}bp_clearvalues_unused;

		FramebufferPrivate::FramebufferPrivate(Device* _d, const FramebufferInfo& info)
		{
			d = (DevicePrivate*)_d;
			renderpass = (RenderpassPrivate*)info.rp;
			views.resize(info.views.size);
			for (auto i = 0; i < info.views.size; i++)
				views[i] = (ImageviewPrivate*)info.views.v[i];

			Vec2i size(0);

#if defined(FLAME_VULKAN)
			std::vector<VkImageView> vk_views(views.size());
			for (auto i = 0; i < views.size(); i++)
			{
				if (i == 0)
					size = views[i]->image->size;
				else
					assert(size == views[i]->image->size);

				vk_views[i] = views[i]->v;
			}

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			create_info.width = size.x();
			create_info.height = size.y();
			create_info.layers = 1;
			create_info.renderPass = renderpass->v;
			create_info.attachmentCount = info.views.size;
			create_info.pAttachments = vk_views.data();

			vk_chk_res(vkCreateFramebuffer(d->v, &create_info, nullptr, &v));
#endif
		}

		FramebufferPrivate::~FramebufferPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyFramebuffer(d->v, v, nullptr);
#endif
		}

		Framebuffer* Framebuffer::create(Device * d, const FramebufferInfo & info)
		{
			return new FramebufferPrivate(d, info);
		}

		void Framebuffer::destroy(Framebuffer * f)
		{
			delete (FramebufferPrivate*)f;
		}

		struct Framebuffer$
		{
			void* device$i;
			void* renderpass$i;
			Array<void*> views$i;

			void* out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				FramebufferInfo info;
				info.rp = (Renderpass*)renderpass$i;
				info.views = views$i;
				out$o = Framebuffer::create((Device*)device$i, info);
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				Framebuffer::destroy((Framebuffer*)out$o);
			}

		}bp_framebuffer_unused;

		struct Framebuffers$
		{
			void* device$i;
			void* renderpass$i;
			Array<void*> views$i;
			uint size$i;

			Array<void*> out$o;

			FLAME_GRAPHICS_EXPORTS void initialize$()
			{
				if (device$i && renderpass$i && size$i > 0)
				{
					out$o.size = size$i;
					out$o.v = new void* [size$i];
					assert(views$i.size >= size$i && views$i.size % size$i == 0);
					auto n = views$i.size / size$i;
					for (auto i = 0; i < size$i; i++)
					{
						FramebufferInfo info;
						info.rp = (Renderpass*)renderpass$i;
						info.views.size = n;
						info.views.v = views$i.v + i * n;
						out$o.v[i] = Framebuffer::create((Device*)device$i, info);
					}
				}
			}

			FLAME_GRAPHICS_EXPORTS void finish$()
			{
				for (auto i = 0; i < out$o.size; i++)
					Framebuffer::destroy((Framebuffer*)out$o.v[i]);
				delete[]out$o.v;
			}

		}bp_framebuffers_unused;
	}
}

