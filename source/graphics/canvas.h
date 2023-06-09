#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Canvas
		{
			WindowPtr window;
			std::vector<ImageViewPtr> iv_tars;
			std::vector<FramebufferPtr> fb_tars;
			bool clear_framebuffer = true;

			virtual ~Canvas() {}

			virtual void set_targets(std::span<ImageViewPtr> targets) = 0;
			virtual void bind_window_targets() = 0;

			virtual void add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) = 0;
			virtual void add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) = 0;
			virtual void add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col, float thickness = 0.f, float border = 0.f) = 0;
			virtual void add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs) = 0;
			virtual void add_image_rotated(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, float angle) = 0;

			struct Create
			{
				virtual CanvasPtr operator()(WindowPtr window) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
