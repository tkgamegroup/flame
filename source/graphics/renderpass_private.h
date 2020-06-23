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
			uint _index;
			Format _format;
			bool _clear;
			SampleCount _sample_count;

			RenderpassAttachmentPrivate(uint index, const RenderpassAttachmentInfo& info);

			uint get_index() const override { return _index; }
			Format get_format() const override { return _format; }
			bool get_clear() const override { return _clear; }
			SampleCount get_sample_count() const override { return _sample_count; }
		};

		struct RenderpassSubpassPrivate : RenderpassSubpass
		{
			uint _index;
			std::vector<RenderpassAttachmentPrivate*> _color_attachments;
			std::vector<RenderpassAttachmentPrivate*> _resolve_attachments;
			RenderpassAttachmentPrivate* _depth_attachment = nullptr;

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
			DevicePrivate* _d;

			std::vector<std::unique_ptr<RenderpassAttachmentPrivate>> _attachments;
			std::vector<std::unique_ptr<RenderpassSubpassPrivate>> _subpasses;

#if defined(FLAME_VULKAN)
			VkRenderPass _v;
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
			DevicePrivate* _d;

			RenderpassPrivate* _rp;
			std::vector<ImageviewPrivate*> _views;
#if defined(FLAME_VULKAN)
			VkFramebuffer _v;
#elif defined(FLAME_D3D12)

#endif
			FramebufferPrivate(DevicePrivate* d, RenderpassPrivate* rp, std::span<ImageviewPrivate*> views);
			~FramebufferPrivate();

			void release() override { delete this; }

			Renderpass* get_renderpass() const override { return _rp; }
			uint get_views_count() const override { return _views.size(); }
			Imageview* get_view(uint idx) const override { return _views[idx]; }
		};
	}
}
