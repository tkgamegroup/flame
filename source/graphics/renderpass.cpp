#include "device_private.h"
#include "renderpass_private.h"

namespace flame
{
	namespace graphics
	{
		RenderpassAttachmentPrivate::RenderpassAttachmentPrivate(uint _index, const RenderpassAttachmentInfo& info) :
			index(_index),
			format(info.format),
			clear(info.clear),
			sample_count(info.sample_count)
		{
		}

		RenderpassSubpassPrivate::RenderpassSubpassPrivate(RenderpassPrivate* rp, uint _index, const RenderpassSubpassInfo& info) :
			index(_index)
		{
			color_attachments.resize(info.color_attachments_count);
			for (auto i = 0; i < color_attachments.size(); i++)
				color_attachments[i] = rp->attachments[i].get();
			resolve_attachments.resize(info.resolve_attachments_count);
			for (auto i = 0; i < resolve_attachments.size(); i++)
				resolve_attachments[i] = rp->attachments[i].get();
			if (info.depth_attachment != -1)
				depth_attachment = rp->attachments[info.depth_attachment].get();
		}

		RenderpassPrivate::RenderpassPrivate(DevicePrivate* d, std::span<const RenderpassAttachmentInfo> _attachments, std::span<const RenderpassSubpassInfo> _subpasses, std::span<const Vec2u> _dependencies) :
			device(d)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkAttachmentDescription> attachment_descs(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
			{
				auto& src = _attachments[i];
				auto& dst = attachment_descs[i];

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

			std::vector<std::vector<VkAttachmentReference>> color_refs(_subpasses.size());
			std::vector<std::vector<VkAttachmentReference>> resolve_refs(_subpasses.size());
			std::vector<VkAttachmentReference> depth_refs(_subpasses.size());
			std::vector<VkSubpassDescription> subpass_descs(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
			{
				auto& src = _subpasses[i];
				auto& dst = subpass_descs[i];

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
					auto& v = color_refs[i];
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
					auto& v = resolve_refs[i];
					v.resize(src.resolve_attachments_count);
					for (auto j = 0; j < v.size(); j++)
					{
						auto& r = v[j];
						r.attachment = src.resolve_attachments[j];
						r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						attachment_descs[j].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					}
					dst.pResolveAttachments = v.data();
				}
				if (src.depth_attachment != -1)
				{
					auto& r = depth_refs[i];
					r.attachment = src.depth_attachment;
					r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					dst.pDepthStencilAttachment = &r;
				}
			}

			std::vector<VkSubpassDependency> subpass_dependencies(_dependencies.size());
			for (auto i = 0; i < _dependencies.size(); i++)
			{
				auto& src = _dependencies[i];
				auto& dst = subpass_dependencies[i];

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
			create_info.attachmentCount = attachment_descs.size();
			create_info.pAttachments = attachment_descs.data();
			create_info.subpassCount = subpass_descs.size();
			create_info.pSubpasses = subpass_descs.data();
			create_info.dependencyCount = subpass_dependencies.size();
			create_info.pDependencies = subpass_dependencies.data();

			chk_res(vkCreateRenderPass(d->vk_device, &create_info, nullptr, &vk_renderpass));

			attachments.resize(_attachments.size());
			for (auto i = 0; i < _attachments.size(); i++)
				attachments[i].reset(new RenderpassAttachmentPrivate(i, _attachments[i]));
			subpasses.resize(_subpasses.size());
			for (auto i = 0; i < _subpasses.size(); i++)
				subpasses[i].reset(new RenderpassSubpassPrivate(this, i, _subpasses[i]));
#endif
		}

		RenderpassPrivate::~RenderpassPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);
#endif
		}

		Renderpass* Renderpass::create(Device* d, uint attachments_count, const RenderpassAttachmentInfo* attachments, uint subpasses_count, const RenderpassSubpassInfo* subpasses, uint dependency_count, const Vec2u* dependencies)
		{
			return new RenderpassPrivate((DevicePrivate*)d, { attachments, attachments_count }, { subpasses, subpasses_count }, { dependencies, dependency_count });
		}

		FramebufferPrivate::FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, std::span<ImageViewPrivate*> _views) :
			device(d),
			renderpass(rp)
		{
#if defined(FLAME_VULKAN)
			std::vector<VkImageView> raw_views(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				raw_views[i] = _views[i]->vk_image_view;

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			auto first_image_size = _views[0]->image->size;
			create_info.width = first_image_size.x();
			create_info.height = first_image_size.y();
			create_info.layers = 1;
			create_info.renderPass = rp->vk_renderpass;
			create_info.attachmentCount = raw_views.size();
			create_info.pAttachments = raw_views.data();

			chk_res(vkCreateFramebuffer(d->vk_device, &create_info, nullptr, &vk_framebuffer));
#endif

			views.resize(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				views[i] = _views[i];
		}

		FramebufferPrivate::~FramebufferPrivate()
		{
#if defined(FLAME_VULKAN)
			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
#endif
		}

		Framebuffer* Framebuffer::create(Device* d, Renderpass* rp, uint views_count, ImageView* const* views)
		{
			return new FramebufferPrivate((DevicePrivate*)d, (RenderpassPrivate*)rp, { (ImageViewPrivate**)views, views_count });
		}

		//void RenderpassAndFramebufferPrivate(Device* d, uint pass_count, SubpassTargetInfo* const* passes)
		//{
		//	std::vector<RenderpassAttachmentInfo*> rp_attachments;
		//	std::vector<RenderpassSubpassInfo*> rp_subpasses;
		//	std::vector<std::tuple<TargetType, void*, std::unique_ptr<RenderpassAttachmentInfo>, Vec4c>> att_infos;
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

		//			auto att_info = new RenderpassAttachmentInfo;
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
		//		std::vector<ImageView*> fb_views;
		//		for (auto& att_info : att_infos)
		//		{
		//			auto type = std::get<0>(att_info);
		//			auto v = std::get<1>(att_info);
		//			switch (type)
		//			{
		//			case TargetImage:
		//			{
		//				auto view = ImageView::create((Image*)v);
		//				created_views.push_back(view);
		//				fb_views.push_back(view);
		//			}
		//				break;
		//			case TargetImageView:
		//				fb_views.push_back((ImageView*)v);
		//				break;
		//			case TargetImages:
		//			{
		//				auto view = ImageView::create(((Array<Image*>*)v)->at(i));
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

