#include <flame/foundation/blueprint.h>
#include "device_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		RenderpassPrivate::RenderpassPrivate(Device* _d, uint attachment_count, const AttachmentInfo* _attachments, uint subpass_count, const SubpassInfo* _subpasses, uint dependency_count, const Vec2u* _dependencies) :
			d((DevicePrivate*)_d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> vk_attachments(attachment_count);
			for (auto i = 0; i < attachment_count; i++)
			{
				auto& src = _attachments[i];
				auto& dst = vk_attachments[i];

				dst.flags = 0;
				dst.format = to_backend(src.format);
				dst.samples = to_backend(src.sample_count);
				dst.loadOp = src.clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				dst.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				dst.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				dst.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				dst.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

				auto fmt = src.format;
				if (fmt >= Format_Color_Begin && fmt <= Format_Color_End)
				{
					if (fmt >= Format_Swapchain_Begin && fmt <= Format_Swapchain_End)
						dst.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
					else
						dst.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				}
				else
					dst.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			std::vector<std::vector<VkAttachmentReference>> vk_color_refs(subpass_count);
			std::vector<std::vector<VkAttachmentReference>> vk_resolve_refs(subpass_count);
			std::vector<VkAttachmentReference> vk_depth_refs(subpass_count);
			std::vector<VkSubpassDescription> vk_subpasses(subpass_count);
			for (auto i = 0; i < subpass_count; i++)
			{
				auto& src = _subpasses[i];
				auto& dst = vk_subpasses[i];

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
				if (src.color_attachment_count)
				{
					auto& v = vk_color_refs[i];
					v.resize(src.color_attachment_count);
					for (auto j = 0; j < v.size(); j++)
					{
						auto& r = v[j];
						r.attachment = src.color_attachments[j];
						r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					dst.colorAttachmentCount = src.color_attachment_count;
					dst.pColorAttachments = v.data();
				}
				if (src.resolve_attachment_count)
				{
					auto& v = vk_resolve_refs[i];
					v.resize(src.resolve_attachment_count);
					for (auto j = 0; j < v.size(); j++)
					{
						auto& r = v[j];
						r.attachment = src.resolve_attachments[j];
						r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						vk_attachments[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					dst.pResolveAttachments = v.data();
				}
				if (src.depth_attachment != -1)
				{
					auto& r = vk_depth_refs[i];
					r.attachment = src.depth_attachment;
					r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					dst.pDepthStencilAttachment = &r;
				}
			}

			std::vector<VkSubpassDependency> vk_dependencies(dependency_count);
			for (auto i = 0; i < dependency_count; i++)
			{
				auto& src = _dependencies[i];
				auto& dst = vk_dependencies[i];

				dst.srcSubpass = src[0];
				dst.dstSubpass = src[1];
				dst.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dst.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
				dst.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dst.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				dst.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
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
				attachments[i] = _attachments[i];
			subpasses.resize(subpass_count);
			for (auto i = 0; i < subpass_count; i++)
				subpasses[i] = _subpasses[i];
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

		Renderpass* Renderpass::create(Device* d, uint attachment_count, const AttachmentInfo* attachments, uint subpass_count, const SubpassInfo* subpasses, uint dependency_count, const Vec2u* dependencies)
		{
			return new RenderpassPrivate(d, attachment_count, attachments, subpass_count, subpasses, dependency_count, dependencies);
		}

		void Renderpass::destroy(Renderpass* r)
		{
			delete (RenderpassPrivate*)r;
		}

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

		//void RenderpassAndFramebufferPrivate(Device* d, uint pass_count, SubpassTargetInfo* const* passes)
		//{
		//	std::vector<AttachmentInfo*> rp_attachments;
		//	std::vector<SubpassInfo*> rp_subpasses;
		//	std::vector<std::tuple<TargetType, void*, std::unique_ptr<AttachmentInfo>, Vec4c>> att_infos;
		//	std::vector<std::unique_ptr<SubpassInfoPrivate>> sp_infos;
		//	for (auto i = 0; i < pass_count; i++)
		//	{
		//		auto find_or_add_att = [&](const RenderTarget& t) {
		//			for (auto i = 0; i < att_infos.size(); i++)
		//			{
		//				auto& att = att_infos[i];
		//				if (t.type == std::get<0>(att) && t.v == std::get<1>(att))
		//					return i;
		//			}

		//			auto idx = (int)att_infos.size();

		//			auto image = image_from_target(t.type, t.v);
		//			assert(image);

		//			auto att_info = new AttachmentInfo;
		//			att_info->format = image->format;
		//			att_info->clear = t.clear;
		//			att_info->sample_count = image->sample_count;
		//			att_infos.emplace_back(t.type, t.v, att_info, t.clear_color);

		//			rp_attachments.push_back(att_info);

		//			return idx;
		//		};

		//		auto src = new SubpassInfoPrivate;
		//		const auto& p = *passes[i];
		//		for (auto i = 0; i < p.color_target_count; i++)
		//			src._color_attachments.push_back(find_or_add_att(*p.color_targets[i]));
		//		for (auto i = 0; i < p.resolve_target_count; i++)
		//			src._resolve_attachments.push_back(find_or_add_att(*p.resolve_targets[i]));
		//		src.color_attachment_count = p.color_target_count;
		//		src.color_attachments = src._color_attachments.data();
		//		src.resolve_attachment_count = p.resolve_target_count;
		//		src.resolve_attachments = src._resolve_attachments.data();
		//		if (p.depth_target)
		//			src.depth_attachment = find_or_add_att(*p.depth_target);
		//		else
		//			src.depth_attachment = -1;

		//		sp_infos.emplace_back(src);
		//		rp_subpasses.push_back(src);

		//	}
		//	rp = (RenderpassPrivate*)Renderpass::create(d, rp_attachments.size(), rp_attachments.data(), rp_subpasses.size(), rp_subpasses.data(), 0, nullptr);

		//	cv = (ClearvaluesPrivate*)Clearvalues::create(rp);
		//	for (auto i = 0; i < att_infos.size(); i++)
		//		cv->set(i, std::get<3>(att_infos[i]));

		//	auto image_count = 0;
		//	for (auto& att_info : att_infos)
		//	{
		//		auto type = std::get<0>(att_info);
		//		if (type == TargetImages)
		//		{
		//			auto count = ((Array<Image*>*)std::get<1>(att_info))->s;
		//			if (image_count == 0)
		//				image_count = count;
		//			else
		//				assert(image_count == count);
		//		}
		//	}
		//	if (image_count == 0)
		//		image_count = 1;

		//	for (auto i = 0; i < image_count; i++)
		//	{
		//		std::vector<Imageview*> fb_views;
		//		for (auto& att_info : att_infos)
		//		{
		//			auto type = std::get<0>(att_info);
		//			auto v = std::get<1>(att_info);
		//			switch (type)
		//			{
		//			case TargetImage:
		//			{
		//				auto view = Imageview::create((Image*)v);
		//				created_views.push_back(view);
		//				fb_views.push_back(view);
		//			}
		//				break;
		//			case TargetImageview:
		//				fb_views.push_back((Imageview*)v);
		//				break;
		//			case TargetImages:
		//			{
		//				auto view = Imageview::create(((Array<Image*>*)v)->at(i));
		//				created_views.push_back(view);
		//				fb_views.push_back(view);
		//			}
		//				break;
		//			}
		//		}
		//		fbs.emplace_back(Framebuffer::create(d, rp, fb_views.size(), fb_views.data()));
		//	}
		//}
	}
}

