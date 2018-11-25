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

#include <flame/ui/widget.h>
#include <flame/ui/style.h>

namespace flame
{
	namespace ui
	{
		FLAME_REGISTER_FUNCTION_BEG(Style_size, FLAME_GID(24726), "p")
			auto &w = *(Widget**)&d[0].p();

			if (w->closet_id$ != d[1].i1())
				return;

			switch (w->state)
			{
			case StateNormal: case StateHovering:
				w->background_offset$ = d[2].f4();
				break;
			case StateActive:
				w->background_offset$ = d[3].f4();
				break;
			}
		FLAME_REGISTER_FUNCTION_END(Style_size)

		void add_style_size(Widget *w, int closet_idx, const Vec2 &minus)
		{
			auto normal_offset = w->background_offset$;
			auto active_offset = normal_offset - Vec4(minus.x, minus.y, minus.x, minus.y);
			
			w->add_style(Style_size::v, { closet_idx, normal_offset, active_offset });
		}

		FLAME_REGISTER_FUNCTION_BEG(Style_color, FLAME_GID(13741), "p")
			auto &w = *(Widget**)&d[0].p();

			if (w->closet_id$ != d[1].i1())
				return;

			switch (w->state)
			{
			case StateNormal:
				w->background_col$ = d[2].b4();
				break;
			case StateHovering:
				w->background_col$ = d[3].b4();
				break;
			case StateActive:
				w->background_col$ = d[4].b4();
				break;
			}
		FLAME_REGISTER_FUNCTION_END(Style_color)

		void add_style_color(Widget *w, int closet_idx, const Vec3 &tint_hsv)
		{
			auto tint_b = HSV(tint_hsv.x, tint_hsv.y, tint_hsv.z, 0.f);
			auto tint = Vec3(tint_b.x / 255.f, tint_b.y / 255.f, tint_b.z / 255.f);

			auto bg_col = w->background_col$;
			bg_col.x *= tint.x;
			bg_col.y *= tint.y;
			bg_col.z *= tint.z;
			auto bg_hsv = to_HSV(bg_col);
			auto bg_normal_col = HSV(bg_hsv.x, bg_hsv.y, bg_hsv.z * 0.9f, bg_col.w / 255.f);
			auto bg_hovering_col = HSV(bg_hsv.x, bg_hsv.y, bg_hsv.z, 1.f);
			auto bg_active_col = HSV(bg_hsv.x, bg_hsv.y, bg_hsv.z * 0.95f, 1.f);

			w->add_style(Style_color::v, { closet_idx, bg_normal_col, bg_hovering_col, bg_active_col });
		}

		FLAME_REGISTER_FUNCTION_BEG(Style_textcolor, FLAME_GID(785), "p")
			auto &w = *(wText**)&d[0].p();

			if (w->closet_id$ != d[1].i1())
				return;

			switch (w->state)
			{
			case StateNormal:
				w->text_col() = d[2].b4();
				break;
			case StateHovering: case StateActive:
				w->text_col() = d[3].b4();
				break;
			}
		FLAME_REGISTER_FUNCTION_END(Style_textcolor)

		void add_style_textcolor(Widget *w, int closet_idx, const Bvec4 &normal_col, const Bvec4 &else_col)
		{
			auto bg_normal_col = normal_col;
			auto bg_hovering_or_active_col = else_col;

			w->add_style(Style_textcolor::v, { closet_idx, bg_normal_col, bg_hovering_or_active_col });
		}
	}
}

