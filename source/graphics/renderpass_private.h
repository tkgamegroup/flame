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
			bool clear;
			SampleCount sample_count;

			RenderpassAttachmentPrivate(uint index, const RenderpassAttachmentInfo& info);

			uint get_index() const override { return _index; }
			Format get_format() const override { return _format; }
			bool get_clear() const override { return _clear; }
			SampleCount get_sample_count() const override { return _sample_count; }
		};

		struct RenderpassSubpassPrivate : RenderpassSubpass
		{
			uint index;
			std::vector<RenderpassAttachmentPrivate*> color_attachments;
			std::vector<RenderpassAttachmentPrivate*> resolve_attachments;
			RenderpassAttachmentPrivate* depth_attachment = nullptr;

			RenderpassSubpassPrivate(RenderpassPrivate* rp, uint index, const RenderpassSubpassInfo& info);

			uint get_index() const override { return _index; }
			uint get_color_attachments_count() const override { return _color_attachments.size(); }
			RenderpassAttachment* get_color_attachment(uint idx) const override { return _color_attachments[idx]; }
			uint get_resolve_attachments_count() const override { return _resolve_attachments.size(); }
			RenderpassAttachment* get_resolve_attachment(uint idx) const override { return _resolve_attachments[idx]; }
			RenderpassAttachment* get_depth_attachment() const override { return _depth_attachment; }
		};

		struct RenderpassPrivate : Renderpass
		{
			DevicePrivate* device;

			std::vector<std::unique_ptr<RenderpassAttachmentPrivate>> attachments;
			std::vector<std::unique_ptr<RenderpassSubpassPrivate>> subpasses;

#if defined(FLAME_VULKAN)
			VkRenderPass vk_renderpass;
#endif
			RenderpassPrivate(DevicePrivate* d, std::span<const RenderpassAttachmentInfo> attachments, std::span<const RenderpassSubpassInfo> subpasses, std::span<const Vec2u> dependencies = {});
			~RenderpassPrivate();

			void release() override { delete this; }

			uint get_attachments_count() const override { return _attachments.size(); }
			RenderpassAttachment* get_attachment_info(uint idx) const override { return _attachments[idx].get(); }
			uint get_subpasses_count() const override { return _subpasses.size(); }
			RenderpassSubpass* get_subpass_info(uint idx) const override { return _subpasses[idx].get(); }
		};

		struct FramebufferPrivate : Framebuffer
		{
			DevicePrivate* device;

			RenderpassPrivate* renderpass;
			std::vector<ImageViewPrivate*> views;
#if defined(FLAME_VULKAN)
			VkFramebuffer vk_framebuffer;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, std::span<ImageViewPrivate*> views);
			~FramebufferPrivate();

			void release() override { delete this; }

			Renderpass* get_renderpass() const override { return _rp; }
			uint get_views_count() const override { return _views.size(); }
			ImageView* get_view(uint idx) const override { return _views[idx]; }
		};
	}
}
