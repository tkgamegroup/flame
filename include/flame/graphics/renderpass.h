#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Image;
		struct Imageview;

		struct AttachmentInfo
		{
			Format format;
			bool clear;
			SampleCount sample_count;

			AttachmentInfo() :
				format(Format_R8G8B8A8_UNORM),
				clear(true),
				sample_count(SampleCount_1)
			{
			}
		};

		struct SubpassInfo
		{
			uint color_attachment_count;
			const uint* color_attachments;
			uint resolve_attachment_count;
			const uint* resolve_attachments;
			int depth_attachment;

			SubpassInfo() :
				color_attachment_count(0),
				color_attachments(nullptr),
				resolve_attachment_count(0),
				resolve_attachments(nullptr),
				depth_attachment(-1)
			{
			}
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS uint attachment_count() const;
			FLAME_GRAPHICS_EXPORTS const AttachmentInfo& attachment_info(uint idx) const;
			FLAME_GRAPHICS_EXPORTS uint subpass_count() const;
			FLAME_GRAPHICS_EXPORTS const SubpassInfo& subpass_info(uint idx) const;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, uint attachment_count, const AttachmentInfo* attachments, uint subpass_count, const SubpassInfo* subpasses, uint dependency_count, const Vec2u* dependencies);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpass *r);
		};

		struct Framebuffer
		{
			Vec2u image_size;

			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;

			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, Renderpass* rp, uint view_count, Imageview* const* views);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};
	}
}

