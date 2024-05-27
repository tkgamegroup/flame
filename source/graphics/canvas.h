#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Canvas
		{
			enum StencilState
			{
				StencilStateNone,
				StencilStateWrite,
				StencilStateCompare
			};

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
					SetStencilState,
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
					StencilState stencil_state;
				}data;
			};

			WindowPtr bound_window = nullptr;
			std::vector<ImageViewPtr> iv_tars;
			std::vector<FramebufferPtr> fb_tars;
			bool clear_framebuffer = true;
			vec2 size;

			FontAtlasPtr default_font_atlas = nullptr;

			std::stack<Rect> scissor_stack;
			std::vector<DrawCmd> draw_cmds;
			std::vector<vec2> path;
			StencilState stencil_state = StencilStateNone;

			virtual ~Canvas() {}

			virtual void set_targets(std::span<ImageViewPtr> targets) = 0;
			virtual void bind_window(WindowPtr window) = 0;

			virtual void reset() = 0;

			virtual uint set_translate(const vec2& translate) = 0; // return: cmd idx
			virtual void push_scissor(const Rect& rect) = 0;
			virtual void pop_scissor() = 0;
			virtual void begin_stencil_write() = 0;
			virtual void end_stencil_write() = 0;
			virtual void begin_stencil_compare() = 0;
			virtual void end_stencil_compare() = 0;

			virtual DrawVert* stroke(float thickness, const cvec4& col, bool closed) = 0;
			virtual DrawVert* fill(const cvec4& col) = 0;

			virtual DrawVert*	add_rect(const vec2& a, const vec2& b, float thickness, const cvec4& col = cvec4(255)) = 0;
			virtual DrawVert*	add_rect_filled(const vec2& a, const vec2& b, const cvec4& col = cvec4(255)) = 0;
			virtual DrawVert*	add_rect_rotated(const vec2& a, const vec2& b, float thickness, const cvec4& col = cvec4(255), float angle = 0) = 0;
			virtual DrawVert*	add_rect_filled_rotated(const vec2& a, const vec2& b, const cvec4& col = cvec4(255), float angle = 0) = 0;
			virtual DrawVert*	add_circle(const vec2& p, float radius, float thickness, const cvec4& col = cvec4(255)) = 0;
			virtual DrawVert*	add_circle_filled(const vec2& p, float radius, const cvec4& col = cvec4(255)) = 0;
			virtual void		add_text(FontAtlasPtr font_atlas, uint font_size, const vec2& pos, std::wstring_view str, const cvec4& col = cvec4(255), float thickness = 0.f, float border = 0.f) = 0;
			virtual DrawVert*	add_image(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs = vec4(0.f, 0.f, 1.f, 1.f), const cvec4& tint_col = cvec4(255), SamplerPtr sp = nullptr) = 0;
			virtual DrawVert*	add_image_stretched(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs = vec4(0.f, 0.f, 1.f, 1.f), const vec4& border = vec4(0.f), const vec4& border_uvs = vec4(0.f, 0.f, 1.f, 1.f), const cvec4& tint_col = cvec4(255), SamplerPtr sp = nullptr) = 0;
			virtual DrawVert*	add_image_rotated(ImageViewPtr view, const vec2& a, const vec2& b, const vec4& uvs = vec4(0.f, 0.f, 1.f, 1.f), const cvec4& tint_col = cvec4(255), float angle = 0, SamplerPtr sp = nullptr) = 0;
			virtual DrawVert*	add_image_polygon(ImageViewPtr view, const std::vector<vec2>& pts, const std::vector<vec2>& uvs, const cvec4& tint_col = cvec4(255), SamplerPtr sp = nullptr) = 0;


			virtual void render(int idx, CommandBufferPtr cb) = 0;

			struct Create
			{
				virtual CanvasPtr operator()(bool hdr = false) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
