#pragma once

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct RenderpassPrivate;

		struct RenderpassAttachmentPrivate : RenderpassAttachment
		{
			uint index;
			Format format;
			AttachmentLoadOp load_op;
			SampleCount sample_count;

			RenderpassAttachmentPrivate(uint index, const RenderpassAttachmentInfo& info);

			uint get_index() const override { return index; }
			Format get_format() const override { return format; }
			AttachmentLoadOp get_load_op() const override { return load_op; }
			SampleCount get_sample_count() const override { return sample_count; }
		};

		struct RenderpassSubpassPrivate : RenderpassSubpass
		{
			uint index;
			std::vector<RenderpassAttachmentPrivate*> color_attachments;
			std::vector<RenderpassAttachmentPrivate*> resolve_attachments;
			RenderpassAttachmentPrivate* depth_attachment = nullptr;

			RenderpassSubpassPrivate(RenderpassPrivate* rp, uint index, const RenderpassSubpassInfo& info);

			uint get_index() const override { return index; }
			uint get_color_attachments_count() const override { return color_attachments.size(); }
			RenderpassAttachment* get_color_attachment(uint idx) const override { return color_attachments[idx]; }
			uint get_resolve_attachments_count() const override { return resolve_attachments.size(); }
			RenderpassAttachment* get_resolve_attachment(uint idx) const override { return resolve_attachments[idx]; }
			RenderpassAttachment* get_depth_attachment() const override { return depth_attachment; }
		};

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* device;

			std::vector<std::unique_ptr<RenderpassAttachmentPrivate>> attachments;
			std::vector<std::unique_ptr<RenderpassSubpassPrivate>> subpasses;

			VkRenderPass vk_renderpass;

			RenderpassPrivate(DevicePrivate* device, std::span<const RenderpassAttachmentInfo> attachments, std::span<const RenderpassSubpassInfo> subpasses, std::span<const uvec2> dependencies = {});
			~RenderpassPrivate();

			void release() override { delete this; }

			uint get_attachments_count() const override { return attachments.size(); }
			RenderpassAttachment* get_attachment_info(uint idx) const override { return attachments[idx].get(); }
			uint get_subpasses_count() const override { return subpasses.size(); }
			RenderpassSubpass* get_subpass_info(uint idx) const override { return subpasses[idx].get(); }
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* device;

			RenderpassPrivate* renderpass;
			std::vector<ImageViewPrivate*> views;
			VkFramebuffer vk_framebuffer;

			FramebufferPrivate(DevicePrivate* device, RenderpassPrivate* rp, std::span<ImageViewPrivate*> views);
			~FramebufferPrivate();

			void release() override { delete this; }

			Renderpass* get_renderpass() const override { return renderpass; }
			uint get_views_count() const override { return views.size(); }
			ImageView* get_view(uint idx) const override { return views[idx]; }
		};
	}
}
