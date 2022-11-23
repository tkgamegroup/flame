#include "../xml.h"
#include "../foundation/typeinfo.h"
#include "../foundation/typeinfo_serialize.h"
#include "device_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		std::vector<std::unique_ptr<RenderpassT>> loaded_renderpasses;

		RenderpassPrivate::~RenderpassPrivate()
		{
			if (app_exiting) return;

			vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);
			unregister_object(vk_renderpass);
		}

		struct RenderpassCreate : Renderpass::Create
		{
			RenderpassPtr operator()(const RenderpassInfo& info) override
			{
				auto ret = new RenderpassPrivate;
				*(RenderpassInfo*)ret = info;

				std::vector<VkAttachmentDescription2> vk_atts(info.attachments.size());
				for (auto i = 0; i < info.attachments.size(); i++)
				{
					auto& src = info.attachments[i];
					auto& dst = vk_atts[i];

					dst = {};
					dst.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
					dst.format = to_backend(src.format);
					dst.samples = to_backend(src.sample_count);
					dst.loadOp = to_backend(src.load_op);
					dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
					dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
					dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
					dst.initialLayout = to_backend(src.initia_layout, src.format);
					dst.finalLayout = to_backend(src.final_layout, src.format);
				}

				struct vkSubpassData
				{
					std::vector<VkAttachmentReference2> col_refs;
					std::vector<VkAttachmentReference2> col_res_refs;
					VkAttachmentReference2 dep_ref;
					VkAttachmentReference2 dep_res_ref;
					VkSubpassDescriptionDepthStencilResolve dep_res_state;
				};
				std::vector<vkSubpassData> vk_sp_datas(info.subpasses.size());
				std::vector<VkSubpassDescription2> vk_sps(info.subpasses.size());
				for (auto i = 0; i < info.subpasses.size(); i++)
				{
					auto& src = info.subpasses[i];
					auto& dst = vk_sp_datas[i];
					auto& sp = vk_sps[i];

					sp = {};
					sp.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
					sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

					if (!src.color_attachments.empty())
					{
						dst.col_refs.resize(src.color_attachments.size());
						for (auto j = 0; j < src.color_attachments.size(); j++)
						{
							auto& r = dst.col_refs[j];
							r = {};
							r.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
							r.attachment = src.color_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}

						sp.colorAttachmentCount = src.color_attachments.size();
						sp.pColorAttachments = dst.col_refs.data();
					}
					if (!src.color_resolve_attachments.empty())
					{
						dst.col_res_refs.resize(src.color_resolve_attachments.size());
						for (auto j = 0; j < src.color_resolve_attachments.size(); j++)
						{
							auto& r = dst.col_res_refs[j];
							r = {};
							r.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
							r.attachment = src.color_resolve_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							ret->attachments[j].final_layout = ImageLayoutAttachment;
							vk_atts[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}

						sp.pResolveAttachments = dst.col_res_refs.data();
					}
					if (src.depth_attachment != -1)
					{
						auto& r = dst.dep_ref;
						r = {};
						r.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
						r.attachment = src.depth_attachment;
						r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

						sp.pDepthStencilAttachment = &r;
					}
					if (src.depth_resolve_attachment != -1)
					{
						auto& r = dst.dep_res_ref;
						r.sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
						r.attachment = src.depth_resolve_attachment;
						r.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
						ret->attachments[src.depth_resolve_attachment].final_layout = ImageLayoutGeneral;
						vk_atts[src.depth_resolve_attachment].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

						auto& s = dst.dep_res_state;
						s = {};
						s.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
						s.depthResolveMode = (device->vk_resolve_props.supportedDepthResolveModes & VK_RESOLVE_MODE_AVERAGE_BIT)
							!= 0 ? VK_RESOLVE_MODE_AVERAGE_BIT : VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
						s.pDepthStencilResolveAttachment = &r;

						sp.pNext = &s;
					}
				}

				std::vector<VkSubpassDependency2> vk_deps(info.dependencies.size());
				for (auto i = 0; i < info.dependencies.size(); i++)
				{
					auto& src = info.dependencies[i];
					auto& dst = vk_deps[i];

					dst = {};
					dst.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
					dst.srcSubpass = src[0];
					dst.dstSubpass = src[1];
					dst.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
					dst.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
					dst.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
					dst.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
					dst.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				}

				VkRenderPassCreateInfo2 create_info;
				create_info = {};
				create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
				create_info.attachmentCount = vk_atts.size();
				create_info.pAttachments = vk_atts.data();
				create_info.subpassCount = vk_sps.size();
				create_info.pSubpasses = vk_sps.data();
				create_info.dependencyCount = vk_deps.size();
				create_info.pDependencies = vk_deps.data();

				chk_res(vkCreateRenderPass2(device->vk_device, &create_info, nullptr, &ret->vk_renderpass));
				register_object(ret->vk_renderpass, "Renderpass", ret);

				return ret;
			}

			RenderpassPtr operator()(const std::string& content, const std::vector<std::string>& defines, const std::string& filename) override
			{
				return nullptr;
			}
		}Renderpass_create;
		Renderpass::Create& Renderpass::create = Renderpass_create;

		struct RenderpassGet : Renderpass::Get
		{
			RenderpassPtr operator()(const std::filesystem::path& _filename, const std::vector<std::string>& defines) override
			{
				auto filename = Path::get(_filename);
				filename.make_preferred();

				for (auto& rp : loaded_renderpasses)
				{
					if (rp->filename == filename && rp->defines == defines)
						return rp.get();
				}

				std::ifstream file(filename);
				if (!file.good())
				{
					wprintf(L"cannot find renderpass: %s\n", _filename.c_str());
					return nullptr;
				}
				LineReader res(file);
				res.read_block("");

				RenderpassInfo info;
				std::vector<std::pair<std::string, std::string>> splited_defines;
				for (auto& d : defines)
				{
					auto sp = SUS::split(d, '=');
					splited_defines.emplace_back(sp[0], sp[1]);
				}
				unserialize_text(res, &info, {}, splited_defines);
				file.close();

				auto ret = Renderpass::create(info);
				ret->filename = filename;
				ret->defines = defines;
				loaded_renderpasses.emplace_back(ret);
				return ret;
			}
		}Renderpass_get;
		Renderpass::Get& Renderpass::get = Renderpass_get;

		FramebufferPrivate::~FramebufferPrivate()
		{
			if (app_exiting) return;

			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
			unregister_object(vk_framebuffer);
		}

		struct FramebufferCreate : Framebuffer::Create
		{
			FramebufferPtr operator()(RenderpassPtr renderpass, std::span<ImageViewPtr> views) override
			{
				auto ret = new FramebufferPrivate;
				ret->renderpass = renderpass;

				std::vector<VkImageView> vk_views(views.size());
				for (auto i = 0; i < views.size(); i++)
					vk_views[i] = views[i]->vk_image_view;

				VkFramebufferCreateInfo create_info;
				create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				create_info.flags = 0;
				create_info.pNext = nullptr;
				auto first_view = views[0];
				auto ext = first_view->image->levels[first_view->sub.base_level].extent;
				create_info.width = ext.x;
				create_info.height = ext.y;
				create_info.layers = 1;
				create_info.renderPass = renderpass->vk_renderpass;
				create_info.attachmentCount = vk_views.size();
				create_info.pAttachments = vk_views.data();

				chk_res(vkCreateFramebuffer(device->vk_device, &create_info, nullptr, &ret->vk_framebuffer));
				register_object(ret->vk_framebuffer, "Framebuffer", ret);

				ret->views.assign(views.begin(), views.end());
				return ret;
			}
		}Framebuffer_create;
		Framebuffer::Create& Framebuffer::create = Framebuffer_create;
	}
}

