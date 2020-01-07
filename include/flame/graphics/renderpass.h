#pragma once

#include <flame/foundation/foundation.h>
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
			Format$ format;
			bool clear;
			SampleCount$ sample_count;
		};

		struct SubpassInfo
		{
			uint color_attachment_count;
			const uint* color_attachments;
			uint resolve_attachment_count;
			const uint* resolve_attachments;
			int depth_attachment;
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS uint attachment_count() const;
			FLAME_GRAPHICS_EXPORTS const AttachmentInfo& attachment_info(uint idx) const;
			FLAME_GRAPHICS_EXPORTS uint subpass_count() const;
			FLAME_GRAPHICS_EXPORTS const SubpassInfo& subpass_info(uint idx) const;

			FLAME_GRAPHICS_EXPORTS static Renderpass* create(Device *d, uint attachment_count, const AttachmentInfo* attachments, uint subpass_count, const SubpassInfo* subpasses, uint dependency_count, const Vec<2, uint>* dependencies);
			FLAME_GRAPHICS_EXPORTS static void destroy(Renderpass *r);
		};

		struct Clearvalues
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;

			FLAME_GRAPHICS_EXPORTS void set(uint idx, const Vec4c &col);

			FLAME_GRAPHICS_EXPORTS static Clearvalues* create(Renderpass* r);
			FLAME_GRAPHICS_EXPORTS static void destroy(Clearvalues* c);
		};

		struct Framebuffer
		{
			Vec2u image_size;

			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;

			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, Renderpass* rp, uint view_count, Imageview* const* views);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};

		struct RenderTarget
		{
			TargetType$ type;
			void* v;
			bool clear;
			Vec4c clear_color;
		};

		struct SubpassTargetInfo
		{
			uint color_target_count;
			const RenderTarget* color_targets;
			uint resolve_target_count;
			const RenderTarget* resolve_targets;
			void* depth_target;
		};

		struct RenderpassAndFramebuffer
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;
			FLAME_GRAPHICS_EXPORTS uint framebuffer_count() const;
			FLAME_GRAPHICS_EXPORTS Framebuffer* const* framebuffers() const;
			FLAME_GRAPHICS_EXPORTS Clearvalues* clearvalues() const;

			FLAME_GRAPHICS_EXPORTS static RenderpassAndFramebuffer* create(Device* d, uint pass_count, const SubpassTargetInfo* passes);
			FLAME_GRAPHICS_EXPORTS static void destroy(RenderpassAndFramebuffer* s);
		};
	}
}

