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

			AttachmentInfo() :
				format(Format_R8G8B8A8_UNORM),
				clear(true),
				sample_count(SampleCount_1)
			{
			}
		};

		struct SubpassInfo
		{
			std::vector<uint> color_attachments;
			std::vector<uint> resolve_attachments;
			int depth_attachment;

			SubpassInfo() :
				depth_attachment(-1)
			{
			}
		};

		struct RenderpassInfo
		{
			std::vector<void*> attachments;
			std::vector<void*> subpasses;
			std::vector<Vec<2, uint>> dependencies;
		};

		struct Renderpass
		{
			FLAME_GRAPHICS_EXPORTS uint attachment_count() const;
			FLAME_GRAPHICS_EXPORTS const AttachmentInfo& attachment_info(uint idx) const;
			FLAME_GRAPHICS_EXPORTS uint subpass_count() const;
			FLAME_GRAPHICS_EXPORTS const SubpassInfo& subpass_info(uint idx) const;

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
			std::vector<void*> views;
		};

		struct Framebuffer
		{
			Vec2u image_size;

			FLAME_GRAPHICS_EXPORTS static Framebuffer* create(Device* d, const FramebufferInfo& info);
			FLAME_GRAPHICS_EXPORTS static void destroy(Framebuffer* f);
		};

		enum SubpassTargetType
		{
			SubpassTargetImage, // v is Image*
			SubpassTargetImageview, // v is Imageview*
			SubpassTargetImages // v is std::vector<Image*>*
		};

		struct SubpassTarget
		{
			SubpassTargetType type;
			void* v;
			bool clear;
			Vec4c clear_color;

			SubpassTarget()
			{
			}

			SubpassTarget(Image* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(SubpassTargetImage),
				v(v),
				clear(clear),
				clear_color(clear_color)
			{
			}

			SubpassTarget(Imageview* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(SubpassTargetImageview),
				v(v),
				clear(clear),
				clear_color(clear_color)
			{
			}

			SubpassTarget(const std::vector<void*>* v, bool clear = false, const Vec4c& clear_color = Vec4c(0)) :
				type(SubpassTargetImages),
				v((void*)v),
				clear(clear),
				clear_color(clear_color)
			{
			}
		};

		struct SubpassTargetInfo
		{
			std::vector<void*> color_targets;
			std::vector<void*> resolve_targets;
			void* depth_target;

			SubpassTargetInfo()
			{
				depth_target = nullptr;
			}
		};

		struct RenderpassAndFramebuffer
		{
			FLAME_GRAPHICS_EXPORTS Renderpass* renderpass() const;
			FLAME_GRAPHICS_EXPORTS const std::vector<void*>& framebuffers() const;
			FLAME_GRAPHICS_EXPORTS Clearvalues* clearvalues() const;

			FLAME_GRAPHICS_EXPORTS static RenderpassAndFramebuffer* create(Device* d, const std::vector<void*>& passes);
			FLAME_GRAPHICS_EXPORTS static void destroy(RenderpassAndFramebuffer* s);
		};
	}
}

