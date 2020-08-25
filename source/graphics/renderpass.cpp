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
				dst.initialLayout = to_backend(src.initia_layout, src.format);
				dst.finalLayout = to_backend(src.final_layout, src.format);
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
		}

		RenderpassPrivate::~RenderpassPrivate()
		{
			vkDestroyRenderPass(device->vk_device, vk_renderpass, nullptr);
		}

		Renderpass* Renderpass::create(Device* d, uint attachments_count, const RenderpassAttachmentInfo* attachments, uint subpasses_count, const RenderpassSubpassInfo* subpasses, uint dependency_count, const Vec2u* dependencies)
		{
			return new RenderpassPrivate((DevicePrivate*)d, { attachments, attachments_count }, { subpasses, subpasses_count }, { dependencies, dependency_count });
		}

		FramebufferPrivate::FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, std::span<ImageViewPrivate*> _views) :
			device(d),
			renderpass(rp)
		{
			std::vector<VkImageView> raw_views(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				raw_views[i] = _views[i]->vk_image_view;

			VkFramebufferCreateInfo create_info;
			create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			create_info.flags = 0;
			create_info.pNext = nullptr;
			auto first_view = _views[0];
			auto lv = first_view->subresource.base_level;
			auto size = first_view->image->size;
			size.x() = max(1U, size.x() >> lv);
			size.y() = max(1U, size.y() >> lv);
			create_info.width = size.x();
			create_info.height = size.y();
			create_info.layers = 1;
			create_info.renderPass = rp->vk_renderpass;
			create_info.attachmentCount = raw_views.size();
			create_info.pAttachments = raw_views.data();

			chk_res(vkCreateFramebuffer(d->vk_device, &create_info, nullptr, &vk_framebuffer));

			views.resize(_views.size());
			for (auto i = 0; i < _views.size(); i++)
				views[i] = _views[i];
		}

		FramebufferPrivate::~FramebufferPrivate()
		{
			vkDestroyFramebuffer(device->vk_device, vk_framebuffer, nullptr);
		}

		Framebuffer* Framebuffer::create(Device* d, Renderpass* rp, uint views_count, ImageView* const* views)
		{
			return new FramebufferPrivate((DevicePrivate*)d, (RenderpassPrivate*)rp, { (ImageViewPrivate**)views, views_count });
		}
	}
}

