#pragma once

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Imageview;

		struct AttachmentInfo
		{
			Format$ format;
			bool clear;
			SampleCount$ sample_count;

			AttachmentInfo() :
				format(Format_R8G8B8A8_UNORM),
				clear(true),
				sample_count(SampleCount_1)
			{
			}
		};

		struct SubpassInfo
		{
			Array<uint> color_attachments;
			Array<uint> resolve_attachments;
			int depth_attachment;

			SubpassInfo() :
				depth_attachment(-1)
			{
				memset(&color_attachments, 0, sizeof(Array<uint>));
				memset(&resolve_attachments, 0, sizeof(Array<uint>));
			}
		};

		struct RenderpassInfo
		{
			Array<void*> attachments;
			Array<void*> subpasses;
			Array<Vec<2, uint>> dependencies;

			RenderpassInfo()
			{
				memset(&attachments, 0, sizeof(Array<AttachmentInfo*>));
				memset(&subpasses, 0, sizeof(Array<SubpassInfo*>));
				memset(&dependencies, 0, sizeof(Array<Vec<2, uint>>));
			}
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS int attachment_count() const;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, const RenderpassInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpass *r);
		};

		struct Clearvalues
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;

			FLAME_GRAPHICS_EXPORTS void set(int idx, const Vec4c &col);

			FLAME_GRAPHICS_EXPORTS static Clearvalues* create(Renderpass* r);
			FLAME_GRAPHICS_EXPORTS static void destroy(Clearvalues* c);
		};

		struct FramebufferInfo
		{
			Renderpass* rp;
			Array<void*> views;
		};

		struct Framebuffer
		{
			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, const FramebufferInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};
	}
}

