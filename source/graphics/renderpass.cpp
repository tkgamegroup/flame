#include "device_private.h"
#include "renderpass_private.h"

#include <flame/foundation/blueprint.h>

#include <memory>

namespace flame
{
	namespace graphics
	{
		RenderpassPrivate::RenderpassPrivate(Device* _d, const std::vector<void*>& _attachments, const std::vector<void*>& _subpasses, const std::vector<Vec<2, uint>>& _dependencies) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
			{
				auto at_info = (AttachmentInfo*)_attachments[i];

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

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(_subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_resolve_refs(_subpasses.size());
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(_subpasses.size());
			std::vector<VkSubpassDescription> vk_subpasses(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
			{
				auto sp_info = (SubpassInfo*)_subpasses[i];

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

			std::vector<VkSubpassDependency> vk_dependencies(_dependencies.size());
			for (auto i = 0; i < _dependencies.size(); i++)
			{
				auto& dp_info = _dependencies[i];

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

			attachments.resize(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
				attachments[i] = *(AttachmentInfo*)(_attachments[i]);
			subpasses.resize(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
				subpasses[i] = *(SubpassInfo*)(_subpasses[i]);

			std::vector<uint> color_attachments;
			for (auto i = 0; i < _subpasses.size(); i++)
			{
				auto sp_info = (SubpassInfo*)_subpasses[i];

				for (auto idx : sp_info->color_attachments)
				{
					auto found = false;
					for (auto existing : color_attachments)
					{
						if (idx == existing)
						{
							found = true;
							break;
						}
					}
					if (!found)
						color_attachments.push_back(idx);
				}
			}
			color_attachment_count = color_attachments.size();
#endif
		}

		RenderpassPrivate::~RenderpassPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyRenderPass(d->v, v, nullptr);
#endif
		}

		uint Renderpass::attachment_count() const
		{
			return ((RenderpassPrivate*)this)->attachments.size();
		}

		uint Renderpass::color_attachment_count() const
		{
			return ((RenderpassPrivate*)this)->color_attachment_count;
		}

		const AttachmentInfo& Renderpass::attachment_info(uint idx) const
		{
			return ((RenderpassPrivate*)this)->attachments[idx];
		}

		uint Renderpass::subpass_count() const
		{
			return ((RenderpassPrivate*)this)->subpasses.size();
		}

		const SubpassInfo& Renderpass::subpass_info(uint idx) const
		{
			return ((RenderpassPrivate*)this)->subpasses[idx];
		}

		Renderpass* Renderpass::create(Device* d, const std::vector<void*>& attachments, const std::vector<void*>& subpasses, const std::vector<Vec<2, uint>>& dependencies)
		{
			return new RenderpassPrivate(d, attachments, subpasses, dependencies);
		}

		void Renderpass::destroy(Renderpass* r)
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

		};

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

		};

		struct Renderpass$
		{
			AttributeP<std::vector<void*>> attachments$i;
			AttributeP<std::vector<void*>> subpasses$i;
			AttributeP<std::vector<Vec<2, uint>>> dependencies$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (attachments$i.frame > out$o.frame || subpasses$i.frame > out$o.frame || dependencies$i.frame > out$o.frame)
				{
					if (out$o.v)
						Renderpass::destroy((Renderpass*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (attachments$i.v && !attachments$i.v->empty() && subpasses$i.v && !subpasses$i.v->empty())
					{
						auto ok = true;
						for (auto a : *attachments$i.v)
						{
							if (((AttachmentInfo*)a)->format == Format_Undefined)
							{
								ok = false;
								break;
							}
						}

						if (ok)
							out$o.v = Renderpass::create(d, *attachments$i.v, *subpasses$i.v, dependencies$i.v ? *dependencies$i.v : std::vector<Vec<2, uint>>());
						else
							out$o.v = nullptr;
					}
					else
					{
						printf("cannot create renderpass\n");

						out$o.v = nullptr;
					}
					out$o.frame = maxN(attachments$i.frame, subpasses$i.frame, dependencies$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Renderpass$()
			{
				if (out$o.v)
					Renderpass::destroy((Renderpass*)out$o.v);
			}

		};

		ClearvaluesPrivate::ClearvaluesPrivate(Renderpass* _rp) :
			rp((RenderpassPrivate*)_rp)
		{
			for (auto i = 0; i < rp->attachments.size(); i++)
			{
				auto fmt = rp->attachment_info(i).format;
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
			return ((ClearvaluesPrivate*)this)->rp;
		}

		void Clearvalues::set(int idx, const Vec4c& col)
		{
			((ClearvaluesPrivate*)this)->set(idx, col);
		}

		Clearvalues* Clearvalues::create(Renderpass* f)
		{
			return new ClearvaluesPrivate(f);
		}

		void Clearvalues::destroy(Clearvalues* c)
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
					if (cv && cv->renderpass())
					{
						auto count = cv->renderpass()->attachment_count();
						for (auto i = 0; i < count; i++)
							cv->set(i, (*colors$i.v)[i]);
					}
					else
						printf("cannot update clearvalues\n");
					out$o.frame = max(out$o.frame, colors$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Clearvalues$()
			{
				if (out$o.v)
					Clearvalues::destroy((Clearvalues*)out$o.v);
			}

		};

		FramebufferPrivate::FramebufferPrivate(Device* _d, Renderpass* _rp, const std::vector<void*>& views) :
			d((DevicePrivate*)_d),
			rp((RenderpassPrivate*)_rp)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkImageView> vk_views(views.size());
			for (auto i = 0; i < views.size(); i++)
			{
				auto v = (ImageviewPrivate*)views[i];

				if (i == 0)
					image_size = v->image->size;
				else
					assert(image_size == v->image->size);

				vk_views[i] = v->v;
			}

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			create_info.width = image_size.x();
			create_info.height = image_size.y();
			create_info.layers = 1;
			create_info.renderPass = rp->v;
			create_info.attachmentCount = views.size();
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

		Framebuffer* Framebuffer::create(Device* d, Renderpass* rp, const std::vector<void*>& views)
		{
			return new FramebufferPrivate(d, rp, views);
		}

		void Framebuffer::destroy(Framebuffer* f)
		{
			delete (FramebufferPrivate*)f;
		}

		struct Framebuffer$
		{
			AttributeP<void> renderpass$i;
			AttributeP<std::vector<void*>> views$i;

			AttributeP<void> out$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (renderpass$i.frame > out$o.frame || views$i.frame > out$o.frame)
				{
					if (out$o.v)
						Framebuffer::destroy((Framebuffer*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					if (d && renderpass$i.v && views$i.v && !views$i.v->empty())
						out$o.v = Framebuffer::create(d, (Renderpass*)renderpass$i.v, *views$i.v);
					else
					{
						printf("cannot create framebuffer\n");

						out$o.v = nullptr;
					}
					out$o.frame = max(renderpass$i.frame, views$i.frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Framebuffer$()
			{
				if (out$o.v)
					Framebuffer::destroy((Framebuffer*)out$o.v);
			}

		};

		RenderpassAndFramebufferPrivate::RenderpassAndFramebufferPrivate(Device* d, const std::vector<void*>& passes)
		{
			std::vector<void*> rp_attachments;
			std::vector<void*> rp_subpasses;
			std::vector<std::tuple<RenderTargetType$, void*, std::unique_ptr<AttachmentInfo>, Vec4c>> att_infos;
			std::vector<std::unique_ptr<SubpassInfo>> sp_infos;
			for (auto& _p : passes)
			{
				auto find_or_add_att = [&](void* p) {
					const auto& t = *(RenderTarget*)p;

					for (auto i = 0; i < att_infos.size(); i++)
					{
						auto& att = att_infos[i];
						if (t.type == std::get<0>(att) && t.v == std::get<1>(att))
							return i;
					}

					auto idx = (int)att_infos.size();

					Image* image = nullptr;
					switch (t.type)
					{
					case SubpassTargetImage:
						image = (Image*)t.v;
						break;
					case SubpassTargetImageview:
						image = ((Imageview*)t.v)->image();
						break;
					case SubpassTargetImages:
						image = (*(std::vector<Image*>*)t.v)[0];
						break;
					}
					assert(image);

					auto att_info = new AttachmentInfo;
					att_info->format = image->format;
					att_info->clear = t.clear;
					att_info->sample_count = image->sample_count;
					att_infos.emplace_back(t.type, t.v, att_info, t.clear_color);

					rp_attachments.push_back(att_info);

					return idx;
				};

				auto sp_info = new SubpassInfo;

				const auto& p = *(SubpassTargetInfo*)_p;

				for (auto& t : p.color_targets)
					sp_info->color_attachments.push_back(find_or_add_att(t));
				for (auto& t : p.resolve_targets)
					sp_info->resolve_attachments.push_back(find_or_add_att(t));
				if (p.depth_target)
					sp_info->depth_attachment = find_or_add_att(p.depth_target);

				sp_infos.emplace_back(sp_info);

				rp_subpasses.push_back(sp_info);
			}
			rp = (RenderpassPrivate*)Renderpass::create(d, rp_attachments, rp_subpasses, {});

			cv = (ClearvaluesPrivate*)Clearvalues::create(rp);
			for (auto i = 0; i < att_infos.size(); i++)
				cv->set(i, std::get<3>(att_infos[i]));

			auto image_count = 0;
			for (auto& att_info : att_infos)
			{
				auto type = std::get<0>(att_info);
				if (type == SubpassTargetImages)
				{
					auto count = ((std::vector<Image*>*)std::get<1>(att_info))->size();
					if (image_count == 0)
						image_count = count;
					else
						assert(image_count == count);
				}
			}

			for (auto i = 0; i < image_count; i++)
			{
				std::vector<void*> fb_views;
				for (auto& att_info : att_infos)
				{
					auto type = std::get<0>(att_info);
					auto v = std::get<1>(att_info);
					switch (type)
					{
					case SubpassTargetImage:
					{
						auto view = Imageview::create((Image*)v);
						created_views.push_back(view);
						fb_views.push_back(view);
					}
						break;
					case SubpassTargetImageview:
						fb_views.push_back(v);
						break;
					case SubpassTargetImages:
					{
						auto view = Imageview::create((*(std::vector<Image*>*)v)[i]);
						created_views.push_back(view);
						fb_views.push_back(view);
					}
						break;
					}
				}
				fbs.emplace_back(Framebuffer::create(d, rp, fb_views));
			}
		}

		RenderpassAndFramebufferPrivate::~RenderpassAndFramebufferPrivate()
		{
			Renderpass::destroy(rp);
			for (auto v : created_views)
				Imageview::destroy(v);
			for (auto f : fbs)
				Framebuffer::destroy((Framebuffer*)f);
			Clearvalues::destroy(cv);
		}

		Renderpass* RenderpassAndFramebuffer::renderpass() const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->rp;
		}

		const std::vector<void*>& RenderpassAndFramebuffer::framebuffers() const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->fbs;
		}

		Clearvalues* RenderpassAndFramebuffer::clearvalues() const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->cv;
		}

		RenderpassAndFramebuffer* RenderpassAndFramebuffer::create(Device* d, const std::vector<void*>& passes)
		{
			return new RenderpassAndFramebufferPrivate(d, passes);
		}

		void RenderpassAndFramebuffer::destroy(RenderpassAndFramebuffer* s)
		{
			delete (RenderpassAndFramebufferPrivate*)s;
		}

		struct RenderTarget$
		{
			AttributeE<RenderTargetType$> type$i;
			AttributeV<bool> clear$i;
			AttributeP<void> v$i;
			AttributeV<Vec4c> clear_color$i;

			AttributeV<RenderTarget> out$o;

			FLAME_GRAPHICS_EXPORTS RenderTarget$()
			{
				clear$i.v = false;
				clear_color$i.v = Vec4c(0);
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (type$i.frame > out$o.frame)
					out$o.v.type = type$i.v;
				if (v$i.frame > out$o.frame)
					out$o.v.v = v$i.v;
				if (clear$i.frame > out$o.frame)
					out$o.v.clear = clear$i.v;
				if (clear_color$i.frame > out$o.frame)
					out$o.v.clear_color = clear_color$i.v;
				out$o.frame = maxN(type$i.frame, v$i.frame, clear$i.frame, clear_color$i.frame);
			}
		};

		struct SubpassTargetInfo$
		{
			AttributeV<std::vector<void*>> color_targets$i;
			AttributeV<std::vector<void*>> resolve_targets$i;
			AttributeP<void> depth_target$i;

			AttributeV<SubpassTargetInfo> out$o;

			FLAME_GRAPHICS_EXPORTS SubpassTargetInfo$()
			{
				depth_target$i.v = nullptr;
			}

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (color_targets$i.frame > out$o.frame)
					out$o.v.color_targets = color_targets$i.v;
				if (resolve_targets$i.frame > out$o.frame)
					out$o.v.resolve_targets = resolve_targets$i.v;
				if (depth_target$i.frame > out$o.frame)
					out$o.v.depth_target = depth_target$i.v;
				out$o.frame = maxN(color_targets$i.frame, resolve_targets$i.frame, depth_target$i.frame);
			}
		};

		struct RenderpassAndFramebuffer$
		{
			AttributeP<std::vector<void*>> passes$i;

			AttributeP<void> out$o;
			AttributeP<void> rp$o;
			AttributeV<std::vector<void*>> fbs$o;
			AttributeP<void> cv$o;

			FLAME_GRAPHICS_EXPORTS void update$()
			{
				if (passes$i.frame > out$o.frame)
				{
					if (out$o.v)
						RenderpassAndFramebuffer::destroy((RenderpassAndFramebuffer*)out$o.v);
					auto d = (Device*)bp_environment().graphics_device;
					auto ok = false;
					if (passes$i.v && !passes$i.v->empty())
					{
						ok = true;

						for (auto& _p : *passes$i.v)
						{
							auto check_target = [&](void* p) {
								const auto& t = *(RenderTarget*)p;

								switch (t.type)
								{
								case SubpassTargetImage: case SubpassTargetImageview:
									return t.v != nullptr;
								case SubpassTargetImages:
									return t.v != nullptr && !(((std::vector<Image*>*)t.v)->empty());
								}
							};

							const auto& p = *(SubpassTargetInfo*)_p;

							for (auto& t : p.color_targets)
							{
								if (!check_target(t))
								{
									ok = false;
									break;
								}
							}
							for (auto& t : p.resolve_targets)
							{
								if (!check_target(t))
								{
									ok = false;
									break;
								}
							}
							if (p.depth_target)
							{
								if (!check_target(p.depth_target))
								{
									ok = false;
									break;
								}
							}
						}
					}
					if (ok)
					{
						auto rnf = RenderpassAndFramebuffer::create(d, *passes$i.v);
						out$o.v = rnf;
						rp$o.v = rnf->renderpass();
						fbs$o.v = rnf->framebuffers();
						cv$o.v = rnf->clearvalues();
					}
					else
					{
						printf("cannot create renderpassandframebuffer\n");

						out$o.v = nullptr;
						rp$o.v = nullptr;
						fbs$o.v.clear();
						cv$o.v = nullptr;
					}
					out$o.frame = passes$i.frame;
					rp$o.frame = passes$i.frame;
					fbs$o.frame = passes$i.frame;
					cv$o.frame = passes$i.frame;
				}
			}

			FLAME_GRAPHICS_EXPORTS ~RenderpassAndFramebuffer$()
			{
				if (out$o.v)
					RenderpassAndFramebuffer::destroy((RenderpassAndFramebuffer*)out$o.v);
			}

		};
	}
}

