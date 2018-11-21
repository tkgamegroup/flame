// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "UI.h"

#include <flame/math.h>

namespace flame
{
	struct FontAtlas;

	namespace graphics
	{
		struct Device;
		struct Commandbuffer;
		struct ClearValues;
	}

	namespace ui
	{
		enum DrawCmdType
		{
			DrawCmd2d,
			DrawCmdText,
			DrawCmdTextSdf,
			DrawCmdScissor
		};

		struct SwapchainData;

		struct Canvas
		{
			graphics::ClearValues *clear_values;

			FLAME_UI_EXPORTS void start_cmd(DrawCmdType type, int id);
			FLAME_UI_EXPORTS void path_line_to(const Vec2 &p);
			FLAME_UI_EXPORTS void path_rect(const Vec2 &pos, const Vec2 &size, float round_radius, int round_flags);
			FLAME_UI_EXPORTS void path_arc_to(const Vec2 &center, float radius, int a_min, int a_max);
			FLAME_UI_EXPORTS void path_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, int level = 0);
			FLAME_UI_EXPORTS void clear_path();
			FLAME_UI_EXPORTS void stroke(const Bvec4 &col, float thickness, bool closed);
			FLAME_UI_EXPORTS void stroke_col2(const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, bool closed);
			FLAME_UI_EXPORTS void fill(const Bvec4 &col);

			FLAME_UI_EXPORTS void add_char_stroke(const Vec2 &pos, const Bvec4 &col, wchar_t ch);
			FLAME_UI_EXPORTS void add_char_sdf(const Vec2 &pos, const Bvec4 &col, wchar_t ch, float scale);
			FLAME_UI_EXPORTS void add_text_stroke(const Vec2 &pos, const Bvec4 &col, const wchar_t *text);
			FLAME_UI_EXPORTS void add_text_sdf(const Vec2 &pos, const Bvec4 &col, const wchar_t *text, float scale);
			FLAME_UI_EXPORTS void add_line(const Vec2 &p0, const Vec2 &p1, const Bvec4 &col, float thickness);
			FLAME_UI_EXPORTS void add_triangle_filled(const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Bvec4 &col);
			FLAME_UI_EXPORTS void add_rect(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, float round_radius = 0.f, int round_flags = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE);
			FLAME_UI_EXPORTS void add_rect_col2(const Vec2 &pos, const Vec2 &size, const Bvec4 &inner_col, const Bvec4 &outter_col, float thickness, float round_radius = 0.f, int round_flags = Rect::SideNW | Rect::SideNE | Rect::SideSW | Rect::SideSE);
			FLAME_UI_EXPORTS void add_rect_rotate(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float thickness, const Vec2 &rotate_center, float angle);
			FLAME_UI_EXPORTS void add_rect_filled(const Vec2 &pos, const Vec2 &size, const Bvec4 &col, float round_radius = 0.f, int round_flags = 0);
			FLAME_UI_EXPORTS void add_circle(const Vec2 &center, float radius, const Bvec4 &col, float thickness);
			inline void add_circle_LT(const Vec2 &center, float diameter, const Bvec4 &col, float thickness)
			{
				add_circle(center + diameter * 0.5f, diameter * 0.5f, col, thickness);
			}
			FLAME_UI_EXPORTS void add_circle_filled(const Vec2 &center, float radius, const Bvec4 &col);
			inline void add_circle_filled_LT(const Vec2 &center, float diameter, const Bvec4 &col)
			{
				add_circle_filled(center + diameter * 0.5f, diameter * 0.5f, col);
			}
			FLAME_UI_EXPORTS void add_bezier(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, const Vec2 &p4, const Bvec4 &col, float thickness);
			FLAME_UI_EXPORTS void add_image(const Vec2 &pos, const Vec2 &size, int id, const Vec2 &uv0 = Vec2(0.f), const Vec2 &uv1 = Vec2(1.f), const Bvec4 &tint_col = Bvec4(255));
			FLAME_UI_EXPORTS void add_image_stretch(const Vec2 &pos, const Vec2 &size, int id, const Vec4 &border, const Bvec4 &tint_col = Bvec4(255));
			FLAME_UI_EXPORTS void set_scissor(const Rect &scissor);

			FLAME_UI_EXPORTS graphics::Commandbuffer *get_cb() const;
			FLAME_UI_EXPORTS void record_cb(int swacpchain_image_index);

			FLAME_UI_EXPORTS static Canvas *create(SwapchainData *s);
			FLAME_UI_EXPORTS static void destroy(Canvas *c);
		};
	}
}
