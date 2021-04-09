#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
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
			int color_attachments_count = 0;
			const int* color_attachments = nullptr;
			int resolve_attachments_count = 0;
			const int* resolve_attachments = nullptr;
			int depth_attachment = -1;
		};

		struct Renderpass
		{
			virtual void release() = 0;

			virtual uint get_attachments_count() const = 0;
			virtual void get_attachment_info(uint idx, RenderpassAttachmentInfo* dst) const = 0;
			virtual uint get_subpasses_count() const = 0;
			virtual void get_subpass_info(uint idx, RenderpassSubpassInfo* dst) const = 0;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device* device, 
				uint attachments_count, const RenderpassAttachmentInfo* attachments, 
				uint subpasses_count, const RenderpassSubpassInfo* subpasses, 
				uint dependencies_count = 0, const uvec2* dependencies = nullptr);
			FLAME_GRAPHICS_EXPORTS static Renderpass* get(Device* device, const wchar_t* filename);
		};

		struct Framebuffer
		{
			virtual void release() = 0;

			virtual RenderpassPtr get_renderpass() const = 0;
			virtual uint get_views_count() const = 0;
			virtual ImageViewPtr get_view(uint idx) const = 0;

			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* device, Renderpass* rp, uint views_count, ImageView* const* views);
		};
	}
}

