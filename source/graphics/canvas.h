#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Canvas
		{
			struct DrawVert
			{
				vec2  pos;
				vec2  uv;
				cvec4 col;
			};

			struct DrawCmd
			{
				enum Type
				{
					SetScissor,
					SetTranslate,
					Blit,
					DrawSdf
				};

				Type type = Blit;
				uint idx_cnt = 0;
				DescriptorSetPtr ds = nullptr;

				union
				{
					vec4 rect;
					vec2 translate;
					struct
					{
						float scale;
						float thickness;
						float border;
					}sdf;
				}data;
			};

			WindowPtr window;
			std::vector<ImageViewPtr> iv_tars;
			std::vector<FramebufferPtr> fb_tars;
			bool clear_framebuffer = true;
			vec2 size;

			FontAtlasPtr default_font_atlas = nullptr;

			std::stack<Rect> scissor_stack;
			std::vector<DrawCmd> draw_cmds;
			std::vector<vec2> path;

			virtual ~Canvas() {}

			virtual void set_targets(std::span<ImageViewPtr> targets) = 0;
			virtual void bind_window_targets() = 0;

			virtual uint set_translate(const vec2& translate) = 0; // return: cmd idx
			virtual void push_scissor(const Rect& rect) = 0;
			virtual void pop_scissor() = 0;

			virtual DrawVert*	add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col) = 0;
			virtual DrawVert*	add_rect_filled(const vec2& a, const vec2& b, const cvec4& col) = 0;
			virtual void		add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col, float thickness = 0.f, float border = 0.f) = 0;
			virtual DrawVert*	add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col) = 0;
			virtual DrawVert*	add_image_stretched(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const vec4& border, float scale, const cvec4& tint_col) = 0;
			virtual DrawVert*	add_image_rotated(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs, const cvec4& tint_col, float angle) = 0;

			struct Create
			{
				virtual CanvasPtr operator()(WindowPtr window) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
