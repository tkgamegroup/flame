#pragma once

#include <flame/graphics/renderpass.h>
#include "graphics_private.h"
#include "image_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct RenderpassAttachmentPrivate : RenderpassAttachment
		{
			uint index;
			Format format;
			bool clear;
			SampleCount sample_count;

			uint get_index() const override { return index; }
			Format get_format() const override { return format; }
			bool get_clear() const override { return clear; }
			SampleCount get_sample_count() const override { return sample_count; }
		};

		struct RenderpassSubpassPrivate : RenderpassSubpass
		{
			std::vector<RenderpassAttachmentPrivate*> color_attachments;
			std::vector<RenderpassAttachmentPrivate*> resolve_attachments;
			RenderpassAttachmentPrivate* depth_attachment;

			RenderpassSubpassPrivate();

			uint get_color_attachments_count() const override { return color_attachments.size(); }
			RenderpassAttachment* get_color_attachment(uint idx) const override { return color_attachments[idx]; }
			uint get_resolve_attachments_count() const override { return resolve_attachments.size(); }
			RenderpassAttachment* get_resolve_attachment(uint idx) const override { return resolve_attachments[idx]; }
			RenderpassAttachment* get_depth_attachment() const override { return depth_attachment; }
		};

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* d;

			std::vector<std::unique_ptr<RenderpassAttachmentPrivate>> attachments;
			std::vector<std::unique_ptr<RenderpassSubpassPrivate>> subpasses;

#if defined(FLAME_VULKAN)
			VkRenderPass v;
#endif
			RenderpassPrivate(DevicePrivate* d, std::span<RenderpassAttachmentInfo> attachments, std::span<RenderpassSubpassInfo> subpasses, std::span<Vec2u> dependencies);
			~RenderpassPrivate();

			void release() override;

			uint get_attachments_count() const override { return attachments.size(); }
			RenderpassAttachment* get_attachment_info(uint idx) const override { return attachments[idx].get(); }
			uint get_subpasses_count() const override { return subpasses.size(); }
			RenderpassSubpass* get_subpass_info(uint idx) const override { return subpasses[idx].get(); }
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* d;

			RenderpassPrivate* rp;
			std::vector<ImageviewPrivate*> views;
#if defined(FLAME_VULKAN)
			VkFramebuffer v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, std::span<ImageviewPrivate*> views);
			~FramebufferPrivate();

			void release() override;

			Renderpass* get_renderpass() const override { return rp; }
			uint get_views_count() const override { return views.size(); }
			Imageview* get_view(uint idx) const override { return views[idx]; }
		};
	}
}
