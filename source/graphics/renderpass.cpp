#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "device_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		std::vector<RenderpassPrivate*> __renderpasses;

		RenderpassPrivate::~RenderpassPrivate()
		{
			if (!__renderpasses.empty())
			{
				std::erase_if(__renderpasses, [&](const auto& rp) {
					return rp == this;
					});
			}

			vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);
		}

		struct RenderpassCreate : Renderpass::Create
		{
			RenderpassPtr operator()(DevicePtr device, std::span<Attachment> attachments, std::span<Subpass> subpasses, std::span<uvec2> dependencies) override
			{
				if (!device)
					device = current_device;

				auto ret = new RenderpassPrivate;
				ret->device = device;

				std::vector<VkAttachmentDescription> atts(attachments.size());
				for (auto i = 0; i < attachments.size(); i++)
				{
					auto& src = attachments[i];
					auto& dst = atts[i];

					dst.flags = 0;
					dst.format = to_backend(src.format);
					dst.samples = to_backend(src.sample_count);
					dst.loadOp = to_backend(src.load_op);
					dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dst.initialLayout = to_backend(src.initia_layout, src.format);
					dst.finalLayout = to_backend(src.final_layout, src.format);
				}

				std::vector<std::vector<VkAttachmentReference>> v_col_refs(subpasses.size());
				std::vector<std::vector<VkAttachmentReference>> v_res_refs(subpasses.size());
				std::vector<VkAttachmentReference> v_dep_refs(subpasses.size());
				std::vector<VkSubpassDescription> vk_sps(subpasses.size());
				for (auto i = 0; i < subpasses.size(); i++)
				{
					auto& src = subpasses[i];
					auto& dst = vk_sps[i];

					dst.flags = 0;
					dst.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
					dst.inputAttachmentCount = 0;
					dst.pInputAttachments = nullptr;
					dst.colorAttachmentCount = 0;
					dst.pColorAttachments = nullptr;
					dst.pResolveAttachments = nullptr;
					dst.pDepthStencilAttachment = nullptr;
					dst.preserveAttachmentCount = 0;
					dst.pPreserveAttachments = nullptr;
					if (!src.color_attachments.empty())
					{
						auto& v = v_col_refs[i];
						v.resize(src.color_attachments.size());
						for (auto j = 0; j < v.size(); j++)
						{
							auto& r = v[j];
							r.attachment = src.color_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
						dst.colorAttachmentCount = src.color_attachments.size();
						dst.pColorAttachments = v.data();
					}
					if (!src.resolve_attachments.empty())
					{
						auto& v = v_res_refs[i];
						v.resize(src.resolve_attachments.size());
						for (auto j = 0; j < v.size(); j++)
						{
							auto& r = v[j];
							r.attachment = src.resolve_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							atts[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
						dst.pResolveAttachments = v.data();
					}
					if (src.depth_attachment != -1)
					{
						auto& r = v_dep_refs[i];
						r.attachment = src.depth_attachment;
						r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						dst.pDepthStencilAttachment = &r;
					}
				}

				std::vector<VkSubpassDependency> vk_deps(dependencies.size());
				for (auto i = 0; i < dependencies.size(); i++)
				{
					auto& src = dependencies[i];
					auto& dst = vk_deps[i];

					dst.srcSubpass = src[0];
					dst.dstSubpass = src[1];
					dst.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dst.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dst.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					dst.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					dst.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}

				VkRenderPassCreateInfo create_info;
				create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
				create_info.flags = 0;
				create_info.pNext = nullptr;
				create_info.attachmentCount = atts.size();
				create_info.pAttachments = atts.data();
				create_info.subpassCount = vk_sps.size();
				create_info.pSubpasses = vk_sps.data();
				create_info.dependencyCount = vk_deps.size();
				create_info.pDependencies = vk_deps.data();

				chk_res(vkCreateRenderPass(device->vk_device, &create_info, nullptr, &ret->vk_renderpass));

				ret->attachments.assign(attachments.begin(), attachments.end());
				ret->subpasses.assign(subpasses.begin(), subpasses.end());

				__renderpasses.push_back(ret);
				return ret;
			}
		}Renderpass_create;
		Renderpass::Create& Renderpass::create = Renderpass_create;

		struct RenderpassGet : Renderpass::Get
		{
			RenderpassPtr operator()(DevicePtr device, const std::filesystem::path& _filename) override
			{
				if (!device)
					device = current_device;

				auto filename = _filename;
				filename.make_preferred();

				for (auto& rp : device->rps)
				{
					if (rp->filename == filename)
						return rp.get();
				}

				if (!get_engine_path(filename, L"default_assets\\shaders"))
				{
					wprintf(L"cannot find rp: %s\n", _filename.c_str());
					return nullptr;
				}

				pugi::xml_document doc;
				pugi::xml_node doc_root;

				if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("renderpass"))
				{
					printf("renderpass wrong format: %s\n", _filename.string().c_str());
					return nullptr;
				}

				auto ti_format = TypeInfo::get(TypeEnumSingle, "flame::graphics::Format", tidb);
				auto ti_loadop = TypeInfo::get(TypeEnumSingle, "flame::graphics::AttachmentLoadOp", tidb);
				auto ti_sampcnt = TypeInfo::get(TypeEnumSingle, "flame::graphics::SampleCount", tidb);
				auto ti_imglayout = TypeInfo::get(TypeEnumSingle, "flame::graphics::ImageLayout", tidb);

				std::vector<Attachment> atts;
				auto n_atts = doc_root.child("attachments");
				for (auto n_att : n_atts.children())
				{
					auto& att = atts.emplace_back();
					if (auto a = n_att.attribute("format"); a)
						ti_format->unserialize(a.value(), &att.format);
					if (auto a = n_att.attribute("load_op"); a)
						ti_loadop->unserialize(a.value(), &att.load_op);
					if (auto a = n_att.attribute("sample_count"); a)
						ti_sampcnt->unserialize(a.value(), &att.sample_count);
					if (auto a = n_att.attribute("initia_layout"); a)
						ti_imglayout->unserialize(a.value(), &att.initia_layout);
					if (auto a = n_att.attribute("final_layout"); a)
						ti_imglayout->unserialize(a.value(), &att.final_layout);
				}

				std::vector<std::vector<int>> v_col_refs;
				std::vector<std::vector<int>> v_res_refs;
				std::vector<int> v_dep_refs;
				std::vector<std::vector<int>> v_depens;
				auto n_sps = doc_root.child("subpasses");
				for (auto n_sp : n_sps.children())
				{
					std::vector<int> col_refs;
					if (auto a = n_sp.attribute("color_refs"); a)
					{
						auto sp = SUS::split(a.value(), ',');
						col_refs.resize(sp.size());
						for (auto i = 0; i < sp.size(); i++)
							col_refs[i] = std::stoi(sp[i]);
					}
					v_col_refs.push_back(col_refs);

					std::vector<int> res_refs;
					if (auto a = n_sp.attribute("resolve_refs"); a)
					{
						auto sp = SUS::split(a.value(), ',');
						res_refs.resize(sp.size());
						for (auto i = 0; i < sp.size(); i++)
							res_refs[i] = std::stoi(sp[i]);
					}
					v_res_refs.push_back(res_refs);

					int dep_ref = -1;
					if (auto a = n_sp.attribute("depth_ref"); a)
						dep_ref = std::stoi(a.value());
					v_dep_refs.push_back(dep_ref);

					std::vector<int> depens;
					if (auto a = n_sp.attribute("dependencies"); a)
					{
						auto sp = SUS::split(a.value(), ',');
						depens.resize(sp.size());
						for (auto i = 0; i < sp.size(); i++)
							depens[i] = std::stoi(sp[i]);
					}
					v_depens.push_back(depens);
				}
				std::vector<Subpass> sps(v_col_refs.size());
				std::vector<uvec2> depens;
				for (auto i = 0; i < sps.size(); i++)
				{
					auto& src = sps.emplace_back();
					src.color_attachments = v_col_refs[i];
					src.resolve_attachments = v_res_refs[i];
					src.depth_attachment = v_dep_refs[i];
					for (auto t : v_depens[i])
						depens.push_back(uvec2(t, i));
				}

				auto ret = Renderpass::create(device, atts, sps, depens);
				ret->filename = filename;
				device->rps.emplace_back(ret);
				return ret;
			}
		}Renderpass_get;
		Renderpass::Get& Renderpass::get = Renderpass_get;

		FramebufferPrivate::~FramebufferPrivate()
		{
			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
		}

		struct FramebufferCreate : Framebuffer::Create
		{
			FramebufferPtr operator()(RenderpassPtr renderpass, std::span<ImageViewPtr> views) override
			{
				auto ret = new FramebufferPrivate;
				ret->device = renderpass->device;
				ret->renderpass = renderpass;

				std::vector<VkImageView> vk_views(views.size());
				for (auto i = 0; i < views.size(); i++)
					vk_views[i] = views[i]->vk_image_view;

				VkFramebufferCreateInfo create_info;
				create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				create_info.flags = 0;
				create_info.pNext = nullptr;
				auto first_view = views[0];
				auto size = first_view->image->sizes[first_view->sub.base_level];
				create_info.width = size.x;
				create_info.height = size.y;
				create_info.layers = 1;
				create_info.renderPass = renderpass->vk_renderpass;
				create_info.attachmentCount = vk_views.size();
				create_info.pAttachments = vk_views.data();

				chk_res(vkCreateFramebuffer(ret->device->vk_device, &create_info, nullptr, &ret->vk_framebuffer));

				ret->views.assign(views.begin(), views.end());
				return ret;
			}
		}Framebuffer_create;
		Framebuffer::Create& Framebuffer::create = Framebuffer_create;
	}
}

