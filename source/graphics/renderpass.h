#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Attachment
		{
			Format format = Format_R8G8B8A8_UNORM;
			AttachmentLoadOp load_op = AttachmentLoadClear;
			AttachmentStoreOp store_op = AttachmentStoreStore;
			SampleCount sample_count = SampleCount_1;
			ImageLayout initia_layout = ImageLayoutUndefined;
			ImageLayout final_layout = ImageLayoutAttachment;
		};

		struct Subpass
		{
			std::vector<int> color_attachments;
			std::vector<int> resolve_attachments;
			int depth_attachment = -1;
		};

		struct Renderpass
		{
			std::vector<Attachment> attachments;
			std::vector<Subpass> subpasses;

			std::filesystem::path filename;

			virtual ~Renderpass() {}

			FLAME_GRAPHICS_EXPORTS static RenderpassPtr create(DevicePtr device, std::span<Attachment> attachments, std::span<Subpass> subpasses, std::span<uvec2> dependencies = {});
			FLAME_GRAPHICS_EXPORTS static RenderpassPtr get(DevicePtr device, const std::filesystem::path& filename);
		};

		struct Framebuffer
		{
			RenderpassPtr renderpass;
			std::vector<ImageViewPtr> views;

			virtual ~Framebuffer() {}

			FLAME_GRAPHICS_EXPORTS static FramebufferPtr create(RenderpassPtr renderpass, std::span<ImageViewPtr> views);
		};
	}
}

