#include <flame/foundation/blueprint.h>
#include "device_private.h"
#include "renderpass_private.h"

#include <flame/reflect_macros.h>

namespace flame
{
	namespace graphics
	{
		RenderpassPrivate::RenderpassPrivate(Device* _d, uint attachment_count, AttachmentInfo* const* _attachments, uint subpass_count, SubpassInfo* const* _subpasses, uint dependency_count, const Vec2u* _dependencies) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(attachment_count);
			for (auto i = 0; i < attachment_count; i++)
			{
				auto at_info = _attachments[i];

				vk_attachments[i].flags = 0;
				vk_attachments[i].format = to_backend(at_info->format);
				vk_attachments[i].samples = to_backend(at_info->sample_count);
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

			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_color_refs(subpass_count);
			std::vector<std::unique_ptr<VkAttachmentReference[]>> vk_resolve_refs(subpass_count);
			std::vector<std::unique_ptr<VkAttachmentReference>> vk_depth_refs(subpass_count);
			std::vector<VkSubpassDescription> vk_subpasses(subpass_count);
			for (auto i = 0; i < subpass_count; i++)
			{
				auto sp_info = _subpasses[i];

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
				if (sp_info->color_attachment_count)
				{
					vk_color_refs[i].reset(new VkAttachmentReference[sp_info->color_attachment_count]);
					for (auto j = 0; j < sp_info->color_attachment_count; j++)
					{
						vk_color_refs[i][j].attachment = sp_info->color_attachments[j];
						vk_color_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].colorAttachmentCount = sp_info->color_attachment_count;
					vk_subpasses[i].pColorAttachments = vk_color_refs[i].get();
				}
				if (sp_info->resolve_attachment_count)
				{
					vk_resolve_refs[i].reset(new VkAttachmentReference[sp_info->resolve_attachment_count]);
					for (auto j = 0; j < sp_info->resolve_attachment_count; j++)
					{
						vk_resolve_refs[i][j].attachment = sp_info->resolve_attachments[j];
						vk_resolve_refs[i][j].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						vk_attachments[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					vk_subpasses[i].pResolveAttachments = vk_resolve_refs[i].get();
				}
				if (sp_info->depth_attachment != -1)
				{
					vk_depth_refs[i].reset(new VkAttachmentReference);
					vk_depth_refs[i]->attachment = sp_info->depth_attachment;
					vk_depth_refs[i]->layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					vk_subpasses[i].pDepthStencilAttachment = vk_depth_refs[i].get();
				}
			}

			std::vector<VkSubpassDependency> vk_dependencies(dependency_count);
			for (auto i = 0; i < dependency_count; i++)
			{
				auto& dp_info = _dependencies[i];

				vk_dependencies[i].srcSubpass = dp_info[0];
				vk_dependencies[i].dstSubpass = dp_info[1];
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

			attachments.resize(attachment_count);
			for (auto i = 0; i < attachment_count; i++)
				attachments[i] = *_attachments[i];
			subpasses.resize(subpass_count);
			for (auto i = 0; i < subpass_count; i++)
				subpasses[i] = *_subpasses[i];
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

		Renderpass* Renderpass::create(Device* d, uint attachment_count, AttachmentInfo* const* attachments, uint subpass_count, SubpassInfo* const* subpasses, uint dependency_count, const Vec2u* dependencies)
		{
			return new RenderpassPrivate(d, attachment_count, attachments, subpass_count, subpasses, dependency_count, dependencies);
		}

		void Renderpass::destroy(Renderpass* r)
		{
			delete (RenderpassPrivate*)r;
		}

		struct AttachmentInfo$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Format$, format);
			BP_IN(bool, clear);
			BP_IN(SampleCount$, sample_count);

			BP_OUT_BASE_LINE;
			BP_OUT(AttachmentInfo, out);

			FLAME_GRAPHICS_EXPORTS AttachmentInfo$()
			{
				format$i = Format_R8G8B8A8_UNORM;
				clear$i = true;
			}

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (format_s()->frame() > out_frame)
				{
					out$o.format = format$i;
					out_updated = true;
				}
				if (clear_s()->frame() > out_frame)
				{
					out$o.clear = clear$i;
					out_updated = true;
				}
				if (sample_count_s()->frame() > out_frame)
				{
					out$o.sample_count = sample_count$i;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}

		};

		struct SubpassInfo$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Array<uint>*, color_attachments);
			BP_IN(Array<uint>*, resolve_attachments);
			BP_IN(int, depth_attachment);

			BP_OUT_BASE_LINE;
			BP_OUT(SubpassInfo, out);

			FLAME_GRAPHICS_EXPORTS SubpassInfo$()
			{
				depth_attachment$i = -1;
			}

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (color_attachments_s()->frame() > out_frame)
				{
					out$o.color_attachment_count = color_attachments$i ? color_attachments$i->s : 0;
					out$o.color_attachments = color_attachments$i ? color_attachments$i->v : nullptr;
					out_updated = true;
				}
				if (resolve_attachments_s()->frame() > out_frame)
				{
					out$o.resolve_attachment_count = resolve_attachments$i ? resolve_attachments$i->s : 0;
					out$o.resolve_attachments = resolve_attachments$i ? resolve_attachments$i->v : nullptr;
					out_updated = true;
				}
				if (depth_attachment_s()->frame() > out_frame)
				{
					out$o.depth_attachment = depth_attachment$i;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		struct Renderpass$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Array<AttachmentInfo*>*, attachments);
			BP_IN(Array<SubpassInfo*>*, subpasses);
			BP_IN(Array<Vec2u>*, dependencies);

			BP_OUT_BASE_LINE;
			BP_OUT(Renderpass*, out);

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (attachments_s()->frame() > out_frame || subpasses_s()->frame() > out_frame || dependencies_s()->frame() > out_frame)
				{
					if (out$o)
						Renderpass::destroy(out$o);
					auto d = Device::default_one();
					if (attachments$i->s && subpasses$i->s)
					{
						auto ok = true;
						for (auto i = 0; i < attachments$i->s; i++)
						{
							if (attachments$i->at(i)->format == Format_Undefined)
							{
								ok = false;
								break;
							}
						}

						if (ok)
							out$o = Renderpass::create(d, attachments$i->s, attachments$i->v, subpasses$i->s, subpasses$i->v, dependencies$i->s, dependencies$i->v);
						else
							out$o = nullptr;
					}
					else
					{
						printf("cannot create renderpass\n");

						out$o = nullptr;
					}
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Renderpass$()
			{
				if (out$o)
					Renderpass::destroy((Renderpass*)out$o);
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

		void Clearvalues::set(uint idx, const Vec4c& col)
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
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Renderpass*, renderpass);
			BP_IN(Array<Vec4c>*, colors);

			BP_OUT_BASE_LINE;
			BP_OUT(Clearvalues*, out);

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (renderpass_s()->frame() > out_frame)
				{
					if (out$o)
						Clearvalues::destroy(out$o);
					if (renderpass$i)
						out$o = Clearvalues::create(renderpass$i);
					else
					{
						printf("cannot create clearvalues\n");

						out$o = nullptr;
					}
					out_updated = true;
				}
				if (colors_s()->frame() > out_frame || renderpass_s()->frame() > out_frame)
				{
					if (out$o && out$o->renderpass())
					{
						if (colors$i)
						{
							for (auto i = 0; i < colors$i->s; i++)
								out$o->set(i, colors$i->at(i));
						}
					}
					else
						printf("cannot update clearvalues\n");
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}

			FLAME_GRAPHICS_EXPORTS ~Clearvalues$()
			{
				if (out$o)
					Clearvalues::destroy(out$o);
			}

		};

		FramebufferPrivate::FramebufferPrivate(Device* _d, Renderpass* _rp, uint view_count, Imageview* const* views) :
			d((DevicePrivate*)_d),
			rp((RenderpassPrivate*)_rp)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkImageView> vk_views(view_count);
			for (auto i = 0; i < view_count; i++)
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
			create_info.attachmentCount = view_count;
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

		Renderpass* Framebuffer::renderpass() const
		{
			return ((FramebufferPrivate*)this)->rp;
		}

		Framebuffer* Framebuffer::create(Device* d, Renderpass* rp, uint view_count, Imageview* const* views)
		{
			return new FramebufferPrivate(d, rp, view_count, views);
		}

		void Framebuffer::destroy(Framebuffer* f)
		{
			delete (FramebufferPrivate*)f;
		}

		struct Framebuffer$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Renderpass*, renderpass);
			BP_IN(Array<Imageview*>*, views);

			BP_OUT_BASE_LINE;
			BP_OUT(Framebuffer*, out);

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (renderpass_s()->frame() > out_frame || views_s()->frame() > out_frame)
				{
					if (out$o)
						Framebuffer::destroy(out$o);
					auto d = Device::default_one();
					if (d && renderpass$i && views$i->s)
						out$o = Framebuffer::create(d, renderpass$i, views$i->s, views$i->v);
					else
					{
						printf("cannot create framebuffer\n");

						out$o = nullptr;
					}
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~Framebuffer$()
			{
				if (out$o)
					Framebuffer::destroy(out$o);
			}

		};

		RenderpassAndFramebufferPrivate::RenderpassAndFramebufferPrivate(Device* d, uint pass_count, SubpassTargetInfo* const* passes)
		{
			std::vector<AttachmentInfo*> rp_attachments;
			std::vector<SubpassInfo*> rp_subpasses;
			std::vector<std::tuple<TargetType$, void*, std::unique_ptr<AttachmentInfo>, Vec4c>> att_infos;
			std::vector<std::unique_ptr<SubpassInfoPrivate>> sp_infos;
			for (auto i = 0; i < pass_count; i++)
			{
				auto find_or_add_att = [&](const RenderTarget& t) {
					for (auto i = 0; i < att_infos.size(); i++)
					{
						auto& att = att_infos[i];
						if (t.type == std::get<0>(att) && t.v == std::get<1>(att))
							return i;
					}

					auto idx = (int)att_infos.size();

					auto image = image_from_target(t.type, t.v);
					assert(image);

					auto att_info = new AttachmentInfo;
					att_info->format = image->format;
					att_info->clear = t.clear;
					att_info->sample_count = image->sample_count;
					att_infos.emplace_back(t.type, t.v, att_info, t.clear_color);

					rp_attachments.push_back(att_info);

					return idx;
				};

				auto sp_info = new SubpassInfoPrivate;
				const auto& p = *passes[i];
				for (auto i = 0; i < p.color_target_count; i++)
					sp_info->_color_attachments.push_back(find_or_add_att(*p.color_targets[i]));
				for (auto i = 0; i < p.resolve_target_count; i++)
					sp_info->_resolve_attachments.push_back(find_or_add_att(*p.resolve_targets[i]));
				sp_info->color_attachment_count = p.color_target_count;
				sp_info->color_attachments = sp_info->_color_attachments.data();
				sp_info->resolve_attachment_count = p.resolve_target_count;
				sp_info->resolve_attachments = sp_info->_resolve_attachments.data();
				if (p.depth_target)
					sp_info->depth_attachment = find_or_add_att(*p.depth_target);
				else
					sp_info->depth_attachment = -1;

				sp_infos.emplace_back(sp_info);
				rp_subpasses.push_back(sp_info);

			}
			rp = (RenderpassPrivate*)Renderpass::create(d, rp_attachments.size(), rp_attachments.data(), rp_subpasses.size(), rp_subpasses.data(), 0, nullptr);

			cv = (ClearvaluesPrivate*)Clearvalues::create(rp);
			for (auto i = 0; i < att_infos.size(); i++)
				cv->set(i, std::get<3>(att_infos[i]));

			auto image_count = 0;
			for (auto& att_info : att_infos)
			{
				auto type = std::get<0>(att_info);
				if (type == TargetImages)
				{
					auto count = ((Array<Image*>*)std::get<1>(att_info))->s;
					if (image_count == 0)
						image_count = count;
					else
						assert(image_count == count);
				}
			}
			if (image_count == 0)
				image_count = 1;

			for (auto i = 0; i < image_count; i++)
			{
				std::vector<Imageview*> fb_views;
				for (auto& att_info : att_infos)
				{
					auto type = std::get<0>(att_info);
					auto v = std::get<1>(att_info);
					switch (type)
					{
					case TargetImage:
					{
						auto view = Imageview::create((Image*)v);
						created_views.push_back(view);
						fb_views.push_back(view);
					}
						break;
					case TargetImageview:
						fb_views.push_back((Imageview*)v);
						break;
					case TargetImages:
					{
						auto view = Imageview::create(((Array<Image*>*)v)->at(i));
						created_views.push_back(view);
						fb_views.push_back(view);
					}
						break;
					}
				}
				fbs.emplace_back(Framebuffer::create(d, rp, fb_views.size(), fb_views.data()));
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

		uint RenderpassAndFramebuffer::framebuffer_count() const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->fbs.size();
		}

		Framebuffer* RenderpassAndFramebuffer::framebuffer(uint idx) const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->fbs[idx];
		}

		Clearvalues* RenderpassAndFramebuffer::clearvalues() const
		{
			return ((RenderpassAndFramebufferPrivate*)this)->cv;
		}

		RenderpassAndFramebuffer* RenderpassAndFramebuffer::create(Device* d, uint pass_count, SubpassTargetInfo* const* passes)
		{
			return new RenderpassAndFramebufferPrivate(d, pass_count, passes);
		}

		void RenderpassAndFramebuffer::destroy(RenderpassAndFramebuffer* s)
		{
			delete (RenderpassAndFramebufferPrivate*)s;
		}

		struct RenderTarget$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(TargetType$, type);
			BP_IN(void*, v);
			BP_IN(bool, clear);
			BP_IN(Vec4c, clear_color);

			BP_OUT_BASE_LINE;
			BP_OUT(RenderTarget, out);
			BP_OUT(Image*, first_image);

			FLAME_GRAPHICS_EXPORTS RenderTarget$()
			{
				clear$i = false;
				clear_color$i = Vec4c(0);
			}

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (type_s()->frame() > out_frame || v_s()->frame() > out_frame)
				{
					out$o.type = type$i;
					out$o.v = v$i;
					out_updated = true;
					first_image$o = image_from_target(type$i, v$i);
					first_image_s()->set_frame(frame);
				}
				if (clear_s()->frame() > out_frame)
				{
					out$o.clear = clear$i;
					out_updated = true;
				}
				if (clear_color_s()->frame() > out_frame)
				{
					out$o.clear_color = clear_color$i;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		struct SubpassTargetInfo$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Array<RenderTarget*>*, color_targets);
			BP_IN(Array<RenderTarget*>*, resolve_targets);
			BP_IN(RenderTarget*, depth_target);

			BP_OUT_BASE_LINE;
			BP_OUT(SubpassTargetInfo, out);

			FLAME_GRAPHICS_EXPORTS SubpassTargetInfo$()
			{
				depth_target$i = nullptr;
			}

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				auto out_frame = out_s()->frame();
				auto out_updated = false;
				if (color_targets_s()->frame() > out_frame)
				{
					out$o.color_target_count = color_targets$i ? color_targets$i->s : 0;
					out$o.color_targets = color_targets$i ? color_targets$i->v : nullptr;
					out_updated = true;
				}
				if (resolve_targets_s()->frame() > out_frame)
				{
					out$o.resolve_target_count = resolve_targets$i ? resolve_targets$i->s : 0;
					out$o.resolve_targets = resolve_targets$i ? resolve_targets$i->v : nullptr;
					out_updated = true;
				}
				if (depth_target_s()->frame() > out_frame)
				{
					out$o.depth_target = depth_target$i;
					out_updated = true;
				}
				if (out_updated)
					out_s()->set_frame(frame);
			}
		};

		struct RenderpassAndFramebuffer$
		{
			BP::Node* n;

			BP_IN_BASE_LINE;
			BP_IN(Array<SubpassTargetInfo*>*, passes);

			BP_OUT_BASE_LINE;
			BP_OUT(RenderpassAndFramebuffer*, out);
			BP_OUT(Renderpass*, rp);
			BP_OUT(Array<Framebuffer*>, fbs);
			BP_OUT(Clearvalues*, cv);

			FLAME_GRAPHICS_EXPORTS void update$(uint frame)
			{
				const auto check_target = [&](const RenderTarget& t) {
					switch (t.type)
					{
					case TargetImage: case TargetImageview:
						return t.v != nullptr;
					case TargetImages:
						return t.v != nullptr && ((Array<Image*>*)t.v)->s;
					}
				};
				const auto check_pass = [&](const SubpassTargetInfo& p) {
					for (auto i = 0; i < p.color_target_count; i++)
					{
						if (!check_target(*p.color_targets[i]))
							return false;
					}
					for (auto i = 0; i < p.resolve_target_count; i++)
					{
						if (!check_target(*p.resolve_targets[i]))
							return false;
					}
					if (p.depth_target)
					{
						if (!check_target(*p.depth_target))
							return false;
					}

					return true;
				};
				if (passes_s()->frame() > out_s()->frame())
				{
					if (out$o)
						RenderpassAndFramebuffer::destroy(out$o);
					auto ok = passes$i && passes$i->s;
					if (ok)
					{
						for (auto i = 0; i < passes$i->s; i++)
						{
							if (!check_pass(*passes$i->at(i)))
							{
								ok = false;
								break;
							}
						}
					}
					if (ok)
					{
						auto rnf = RenderpassAndFramebuffer::create(Device::default_one(), passes$i->s, passes$i->v);
						out$o = rnf;
						rp$o = rnf->renderpass();
						fbs$o.resize(rnf->framebuffer_count());
						for (auto i = 0; i < fbs$o.s; i++)
							fbs$o[i] = rnf->framebuffer(i);
						cv$o = rnf->clearvalues();
					}
					else
					{
						printf("cannot create renderpassandframebuffer\n");

						out$o = nullptr;
						rp$o = nullptr;
						fbs$o.resize(0);
						cv$o = nullptr;
					}
					out_s()->set_frame(frame);
					rp_s()->set_frame(frame);
					fbs_s()->set_frame(frame);
					cv_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS ~RenderpassAndFramebuffer$()
			{
				if (out$o)
					RenderpassAndFramebuffer::destroy(out$o);
			}
		};
	}
}

