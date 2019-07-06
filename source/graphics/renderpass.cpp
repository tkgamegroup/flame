#include "device_private.h"
#include "renderpass_private.h"

#include <memory>

namespace flame
{
	namespace graphics
	{
		RenderpassPrivate::RenderpassPrivate(Device *_d, const RenderpassInfo& info)
		{
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(info.attachments.size());
			for (auto i = 0; i < vk_attachments.size(); i++)
			{
				auto at_info = (AttachmentInfo*)info.attachments[i];

				vk_attachments[i].flags = 0;
				vk_attachments[i].format = to_enum(at_info->format);
				vk_attachments[i].samples = to_enum(at_info->sample_count);
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

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(info.subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_resolve_refs(info.subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(info.subpasses.size());
			std::vector<VkSubpassDescription> vk_subpasses(info.subpasses.size());
			for (auto i = 0; i < vk_subpasses.size(); i++)
			{
				auto sp_info = (SubpassInfo*)info.subpasses[i];

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
				if (sp_info->color_attachments.size() > 0)
				{
					vk_color_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[sp_info->color_attachments.size()]);
					for (auto j = 0; j < sp_info->color_attachments.size(); j++)
					{
						vk_color_refs[i][j].attachment = sp_info->color_attachments[j];
						vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].colorAttachmentCount = sp_info->color_attachments.size();
					vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
				}
				if (sp_info->resolve_attachments.size() != 0)
				{
					vk_resolve_refs[i] = std::unique_ptr<VkAttachmentReference[]>(new VkAttachmentReference[sp_info->color_attachments.size()]);
					for (auto j = 0; j < sp_info->color_attachments.size(); j++)
					{
						vk_resolve_refs[i][j].attachment = sp_info->resolve_attachments[j];
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

			std::vector<VkSubpassDependency> vk_dependencies(info.dependencies.size());
			for (auto i = 0; i < info.dependencies.size(); i++)
			{
				auto& dp_info = info.dependencies[i];

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

			chk_res(vkCreateRenderPass(d->v, &create_info, nullptr, &v));

			attachments.resize(info.attachments.size());
			for (auto i = 0; i < info.attachments.size(); i++)
				attachments[i] = ((AttachmentInfo*)(info.attachments[i]))->format;
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

		struct AttachmentInfo$
		{
			AttributeE<Format$> format$i;
			AttributeV<bool> clear$i;
			AttributeE<SampleCount$> sample_count$i;

			AttributeV<AttachmentInfo> out$o;

			FLAME_GRAPHICS_EXPORTS AttachmentInfo$()
			{
				format$i.v = Format_R8G8B8A8_UNORM;
				clear$i.v = true;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (format$i.frame > out$o.frame)
					out$o.v.format = format$i.v;
				if (clear$i.frame > out$o.frame)
					out$o.v.clear = clear$i.v;
				if (sample_count$i.frame > out$o.frame)
					out$o.v.sample_count = sample_count$i.v;
				out$o.frame = maxN(format$i.frame, clear$i.frame, sample_count$i.frame);
			}

		}bp_attachmentinfo_unused;

		struct SubpassInfo$
		{
			AttributeV<std::vector<uint>> color_attachments$i;
			AttributeV<std::vector<uint>> resolve_attachments$i;
			AttributeV<int> depth_attachment$i;

			AttributeV<SubpassInfo> out$o;

			FLAME_GRAPHICS_EXPORTS SubpassInfo$()
			{
				depth_attachment$i.v = -1;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (color_attachments$i.frame > out$o.frame)
					out$o.v.color_attachments = color_attachments$i.v;
				if (resolve_attachments$i.frame > out$o.frame)
					out$o.v.resolve_attachments = resolve_attachments$i.v;
				if (depth_attachment$i.frame > out$o.frame)
					out$o.v.depth_attachment = depth_attachment$i.v;
				out$o.frame = maxN(color_attachments$i.frame, resolve_attachments$i.frame, depth_attachment$i.frame);
			}

		}bp_subpassinfo_unused;

		struct Renderpass$
		{
			AttributeP<void> device$i;
			AttributeP<std::vector<void*>> attachments$i;
			AttributeP<std::vector<void*>> subpasses$i;
			AttributeP<std::vector<Vec<2, uint>>> dependencies$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (device$i.frame > out$o.frame || attachments$i.frame > out$o.frame || subpasses$i.frame > out$o.frame || dependencies$i.frame > out$o.frame)
				{
					if (out$o.v)
						Renderpass::destroy((Renderpass*)out$o.v);
					if (device$i.v && attachments$i.v && !attachments$i.v->empty() && subpasses$i.v && !subpasses$i.v->empty())
					{
						RenderpassInfo info;
						info.attachments = *attachments$i.v;
						info.subpasses = *subpasses$i.v;
						if (dependencies$i.v)
							info.dependencies = *dependencies$i.v;
						out$o.v = Renderpass::create((Device*)device$i.v, info);
					}
					else
					{
						printf("cannot create renderpass\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(device$i.frame, attachments$i.frame, subpasses$i.frame, dependencies$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Renderpass$()
			{
				if (out$o.v)
					Renderpass::destroy((Renderpass*)out$o.v);
			}

		}bp_renderpass_unused;

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

		void ClearvaluesPrivate::set(uint idx, const Vec4c& col)
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
			AttributeP<void> renderpass$i;
			AttributeP<std::vector<Vec4c>> colors$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				auto last_out_frame = out$o.frame;
				if (renderpass$i.frame > last_out_frame)
				{
					if (out$o.v)
						Clearvalues::destroy((Clearvalues*)out$o.v);
					if (renderpass$i.v)
						out$o.v = Clearvalues::create((Renderpass*)renderpass$i.v);
					else
					{
						printf("cannot create clearvalues\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(out$o.frame, renderpass$i.frame);
				}
				if (colors$i.frame > last_out_frame || renderpass$i.frame > last_out_frame)
				{
					auto cv = (Clearvalues*)out$o.v;
					auto count = cv->renderpass()->attachment_count();
					for (auto i = 0; i < count; i++)
						cv->set(i, (*colors$i.v)[i]);
					out$o.frame = max(out$o.frame, colors$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Clearvalues$()
			{
				if (out$o.v)
					Clearvalues::destroy((Clearvalues*)out$o.v);
			}

		}bp_clearvalues_unused;

		FramebufferPrivate::FramebufferPrivate(Device* _d, const FramebufferInfo& info)
		{
			d = (DevicePrivate*)_d;
			renderpass = (RenderpassPrivate*)info.rp;
			views.resize(info.views.size());
			for (auto i = 0; i < info.views.size(); i++)
				views[i] = (ImageviewPrivate*)info.views[i];

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
			create_info.attachmentCount = info.views.size();
			create_info.pAttachments = vk_views.data();

			chk_res(vkCreateFramebuffer(d->v, &create_info, nullptr, &v));
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
			AttributeP<void> device$i;
			AttributeP<void> renderpass$i;
			AttributeP<std::vector<void*>> views$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (device$i.frame > out$o.frame || renderpass$i.frame > out$o.frame || views$i.frame > out$o.frame)
				{
					if (out$o.v)
						Framebuffer::destroy((Framebuffer*)out$o.v);
					if (device$i.v && renderpass$i.v && views$i.v && !views$i.v->empty())
					{
						FramebufferInfo info;
						info.rp = (Renderpass*)renderpass$i.v;
						info.views = *views$i.v;
						out$o.v = Framebuffer::create((Device*)device$i.v, info);
					}
					else
					{
						printf("cannot create framebuffer\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(device$i.frame, renderpass$i.frame, views$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Framebuffer$()
			{
				if (out$o.v)
					Framebuffer::destroy((Framebuffer*)out$o.v);
			}

		}bp_framebuffer_unused;

		struct Framebuffers$
		{
			AttributeP<void> device$i;
			AttributeP<void> renderpass$i;
			AttributeP<std::vector<void*>> views$i;
			AttributeV<uint> size$i;

			AttributeV<std::vector<void*>> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (device$i.frame > out$o.frame || renderpass$i.frame > out$o.frame || views$i.frame > out$o.frame || size$i.frame > out$o.frame)
				{
					for (auto i = 0; i < out$o.v.size(); i++)
						Framebuffer::destroy((Framebuffer*)out$o.v[i]);
					if (device$i.v && renderpass$i.v && views$i.v && !views$i.v->empty() && size$i.v > 0 && views$i.v->size() >= size$i.v && views$i.v->size() % size$i.v == 0)
					{
						out$o.v.resize(size$i.v);
						auto n = views$i.v->size() / size$i.v;
						for (auto i = 0; i < size$i.v; i++)
						{
							FramebufferInfo info;
							info.rp = (Renderpass*)renderpass$i.v;
							info.views.resize(n);
							for (auto j = 0; j < n; j++)
								info.views[j] = (*views$i.v)[i * n + j];
							out$o.v[i] = Framebuffer::create((Device*)device$i.v, info);
						}
					}
					else
					{
						printf("cannot create framebuffers\n");

						out$o.v.clear();
					}
					out$o.frame = maxN(device$i.frame, renderpass$i.frame, views$i.frame, size$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Framebuffers$()
			{
				for (auto i = 0; i < out$o.v.size(); i++)
					Framebuffer::destroy((Framebuffer*)out$o.v[i]);
			}

		}bp_framebuffers_unused;
	}
}

