#include "../foundation/typeinfo.h"
#include "device_private.h"
#include "renderpass_private.h"

#include <pugixml.hpp>

namespace flame
{
	namespace graphics
	{
		RenderpassPrivate::RenderpassPrivate(DevicePrivate* device, std::span<const RenderpassAttachmentInfo> _attachments, std::span<const RenderpassSubpassInfo> _subpasses, std::span<const uvec2> _dependencies) :
			device(device)
		{
			std::vector<VkAttachmentDescription> atts(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
			{
				auto& src = _attachments[i];
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

			std::vector<std::vector<VkAttachmentReference>> v_col_refs(_subpasses.size());
			std::vector<std::vector<VkAttachmentReference>> v_res_refs(_subpasses.size());
			std::vector<VkAttachmentReference> v_dep_refs(_subpasses.size());
			std::vector<VkSubpassDescription> vk_sps(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
			{
				auto& src = _subpasses[i];
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
				if (src.color_attachments_count > 0)
				{
					auto& v = v_col_refs[i];
					v.resize(src.color_attachments_count);
					for (auto j = 0; j < v.size(); j++)
					{
						auto& r = v[j];
						r.attachment = src.color_attachments[j];
						r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					dst.colorAttachmentCount = src.color_attachments_count;
					dst.pColorAttachments = v.data();
				}
				if (src.resolve_attachments_count > 0)
				{
					auto& v = v_res_refs[i];
					v.resize(src.resolve_attachments_count);
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

			std::vector<VkSubpassDependency> vk_deps(_dependencies.size());
			for (auto i = 0; i < _dependencies.size(); i++)
			{
				auto& src = _dependencies[i];
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

			chk_res(vkCreateRenderPass(device->vk_device, &create_info, nullptr, &vk_renderpass));

			attachments.resize(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
				attachments[i] = _attachments[i];
			subpasses.resize(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
			{
				auto& src = _subpasses[i];
				auto& dst = subpasses[i];
				dst.color_attachments_count = src.color_attachments_count;
				if (dst.color_attachments_count)
				{
					dst.color_attachments = new int[dst.color_attachments_count];
					memcpy((void*)dst.color_attachments, src.color_attachments, sizeof(int) * dst.color_attachments_count);
				}
				dst.resolve_attachments_count = src.resolve_attachments_count;
				if (dst.resolve_attachments_count)
				{
					dst.resolve_attachments = new int[dst.resolve_attachments_count];
					memcpy((void*)dst.resolve_attachments, src.resolve_attachments, sizeof(int) * dst.resolve_attachments_count);
				}
				dst.depth_attachment = src.depth_attachment;
			}
		}

		RenderpassPrivate::~RenderpassPrivate()
		{
			for (auto& sp : subpasses)
			{
				delete[]sp.color_attachments;
				delete[]sp.resolve_attachments;
			}
			vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);
		}

		RenderpassPrivate* RenderpassPrivate::get(DevicePrivate* device, const std::filesystem::path& _filename)
		{
			auto filename = _filename;
			if (!get_engine_path(filename, L"assets\\shaders"))
			{
				wprintf(L"cannot find rp: %s\n", _filename.c_str());
				return nullptr;
			}
			filename.make_preferred();

			for (auto& rp : device->rps)
			{
				if (rp->filename == filename)
					return rp.get();
			}

			pugi::xml_document doc;
			pugi::xml_node doc_root;

			if (!doc.load_file(filename.c_str()) || (doc_root = doc.first_child()).name() != std::string("renderpass"))
			{
				printf("renderpass wrong format: %s\n", _filename.string().c_str());
				return nullptr;
			}

			std::vector<RenderpassAttachmentInfo> atts;
			auto n_atts = doc_root.child("attachments");
			for (auto n_att : n_atts.children())
			{
				RenderpassAttachmentInfo att;
				if (auto a = n_att.attribute("format"); a)
					ti_es("flame::graphics::Format")->unserialize(&att.format, a.value());
				if (auto a = n_att.attribute("load_op"); a)
					ti_es("flame::graphics::AttachmentLoadOp")->unserialize(&att.load_op, a.value());
				if (auto a = n_att.attribute("sample_count"); a)
					ti_es("flame::graphics::SampleCount")->unserialize(&att.sample_count, a.value());
				if (auto a = n_att.attribute("initia_layout"); a)
					ti_es("flame::graphics::ImageLayout")->unserialize(&att.initia_layout, a.value());
				if (auto a = n_att.attribute("final_layout"); a)
					ti_es("flame::graphics::ImageLayout")->unserialize(&att.final_layout, a.value());
				atts.push_back(att);
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
			std::vector<RenderpassSubpassInfo> sps(v_col_refs.size());
			std::vector<uvec2> depens;
			for (auto i = 0; i < sps.size(); i++)
			{
				auto& src = sps[i];
				src.color_attachments_count = v_col_refs[i].size();
				src.color_attachments = v_col_refs[i].data();
				src.resolve_attachments_count = v_res_refs[i].size();
				src.resolve_attachments = v_res_refs[i].data();
				src.depth_attachment = v_dep_refs[i];
				for (auto t : v_depens[i])
					depens.push_back(uvec2(t, i));
			}

			auto rp = new RenderpassPrivate(device, atts, sps, depens);
			rp->filename = filename;
			device->rps.emplace_back(rp);
			return rp;
		}

		Renderpass* Renderpass::get(Device* device, const wchar_t* filename)
		{
			return RenderpassPrivate::get((DevicePrivate*)device, filename);
		}

		Renderpass* Renderpass::create(Device* device, uint attachments_count, const RenderpassAttachmentInfo* attachments, uint subpasses_count, const RenderpassSubpassInfo* subpasses, uint dependency_count, const uvec2* dependencies)
		{
			return new RenderpassPrivate((DevicePrivate*)device, { attachments, attachments_count }, { subpasses, subpasses_count }, { dependencies, dependency_count });
		}

		FramebufferPrivate::FramebufferPrivate(DevicePrivate* device, RenderpassPrivate* rp, std::span<ImageViewPrivate*> _views) :
			device(device),
			renderpass(rp)
		{
			std::vector<VkImageView> vk_views(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				vk_views[i] = _views[i]->vk_image_view;

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			auto first_view = _views[0];
			auto size = first_view->image->sizes[first_view->sub.base_level];
			create_info.width = size.x;
			create_info.height = size.y;
			create_info.layers = 1;
			create_info.renderPass = rp->vk_renderpass;
			create_info.attachmentCount = vk_views.size();
			create_info.pAttachments = vk_views.data();

			chk_res(vkCreateFramebuffer(device->vk_device, &create_info, nullptr, &vk_framebuffer));

			views.resize(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				views[i] = _views[i];
		}

		FramebufferPrivate::~FramebufferPrivate()
		{
			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
		}

		Framebuffer* Framebuffer::create(Device* device, Renderpass* rp, uint views_count, ImageView* const* views)
		{
			return new FramebufferPrivate((DevicePrivate*)device, (RenderpassPrivate*)rp, { (ImageViewPrivate**)views, views_count });
		}
	}
}

