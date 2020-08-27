#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Image;
		struct ImageView;

		struct RenderpassAttachmentInfo
		{
			Format format = Format_R8G8B8A8_UNORM;
			AttachmentLoadOp load_op = AttachmentClear;
			SampleCount sample_count = SampleCount_1;
			ImageLayout initia_layout = ImageLayoutUndefined;
			ImageLayout final_layout = ImageLayoutShaderReadOnly;
		};

		struct RenderpassSubpassInfo
		{
			uint color_attachments_count = 0;
			const uint* color_attachments = nullptr;
			uint resolve_attachments_count = 0;
			const uint* resolve_attachments = nullptr;
			int depth_attachment = -1;
		};

		struct RenderpassAttachment
		{
			virtual uint get_index() const = 0;
			virtual Format get_format() const = 0;
			virtual AttachmentLoadOp get_load_op() const = 0;
			virtual SampleCount get_sample_count() const = 0;
		};

		struct RenderpassSubpass
		{
			virtual uint get_index() const = 0;
			virtual uint get_color_attachments_count() const = 0;
			virtual RenderpassAttachment* get_color_attachment(uint idx) const = 0;
			virtual uint get_resolve_attachments_count() const = 0;
			virtual RenderpassAttachment* get_resolve_attachment(uint idx) const = 0;
			virtual RenderpassAttachment* get_depth_attachment() const = 0;
		};

		struct Renderpass
		{
			virtual void release() = 0;

			virtual uint get_attachments_count() const = 0;
			virtual RenderpassAttachment* get_attachment_info(uint idx) const = 0;
			virtual uint get_subpasses_count() const = 0;
			virtual RenderpassSubpass* get_subpass_info(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, 
				uint attachments_count, const RenderpassAttachmentInfo* attachments, 
				uint subpasses_count, const RenderpassSubpassInfo* subpasses, 
				uint dependencies_count = 0, const Vec2u* dependencies = nullptr);
		};

		struct Framebuffer
		{
			virtual void release() = 0;

			virtual Renderpass* get_renderpass() const = 0;
			virtual uint get_views_count() const = 0;
			virtual ImageView* get_view(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, Renderpass* rp, uint views_count, ImageView* const* views);
		};
	}
}

