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
			unregister_backend_object(vk_renderpass);
		}

		struct RenderpassCreate : Renderpass::Create
		{
			RenderpassPtr operator()(const RenderpassInfo& info) override
			{
				auto ret = new RenderpassPrivate;
				*(RenderpassInfo*)ret = info;

				std::vector<VkAttachmentDescription> atts(info.attachments.size());
				for (auto i = 0; i < info.attachments.size(); i++)
				{
					auto& src = info.attachments[i];
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

				struct vkSubpassInfo
				{
					std::vector<VkAttachmentReference> col_refs;
					std::vector<VkAttachmentReference> res_refs;
					VkAttachmentReference dep_ref;
				};
				std::vector<vkSubpassInfo> vk_sp_infos(info.subpasses.size());
				std::vector<VkSubpassDescription> vk_sps(info.subpasses.size());
				for (auto i = 0; i < info.subpasses.size(); i++)
				{
					auto& src = info.subpasses[i];
					auto& dst = vk_sp_infos[i];
					auto& sp = vk_sps[i];

					sp = {};
					sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

					if (!src.color_attachments.empty())
					{
						dst.col_refs.resize(src.color_attachments.size());
						for (auto j = 0; j < src.color_attachments.size(); j++)
						{
							auto& r = dst.col_refs[j];
							r.attachment = src.color_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
						sp.colorAttachmentCount = src.color_attachments.size();
						sp.pColorAttachments = dst.col_refs.data();
					}
					if (!src.resolve_attachments.empty())
					{
						dst.res_refs.resize(src.resolve_attachments.size());
						for (auto j = 0; j < src.resolve_attachments.size(); j++)
						{
							auto& r = dst.res_refs[j];
							r.attachment = src.resolve_attachments[j];
							r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
							atts[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						}
						sp.pResolveAttachments = dst.res_refs.data();
					}
					if (src.depth_attachment != -1)
					{
						auto& r = dst.dep_ref;
						r.attachment = src.depth_attachment;
						r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
						sp.pDepthStencilAttachment = &r;
					}
				}

				std::vector<VkSubpassDependency> vk_deps(info.dependencies.size());
				for (auto i = 0; i < info.dependencies.size(); i++)
				{
					auto& src = info.dependencies[i];
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
				register_backend_object(ret->vk_renderpass, th<decltype(*ret)>(), ret);

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

				if (!std::filesystem::exists(filename))
				{
					wprintf(L"cannot find renderpass: %s\n", _filename.c_str());
					return nullptr;
				}

				RenderpassInfo info;

				std::ifstream file(filename);
				LineReader res(file);
				res.read_block("");
				unserialize_text(res, &info, {}, defines);
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
			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
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
				auto size = first_view->image->levels[first_view->sub.base_level].size;
				create_info.width = size.x;
				create_info.height = size.y;
				create_info.layers = 1;
				create_info.renderPass = renderpass->vk_renderpass;
				create_info.attachmentCount = vk_views.size();
				create_info.pAttachments = vk_views.data();

				chk_res(vkCreateFramebuffer(device->vk_device, &create_info, nullptr, &ret->vk_framebuffer));

				ret->views.assign(views.begin(), views.end());
				return ret;
			}
		}Framebuffer_create;
		Framebuffer::Create& Framebuffer::create = Framebuffer_create;
	}
}

