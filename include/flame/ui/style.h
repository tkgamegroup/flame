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

#include <flame/math.h>
#include <flame/ui/ui.h>

namespace flame
{
	namespace ui
	{
		struct Widget;

		FLAME_UI_EXPORTS void add_style_background_offset(Widget *w, int closet_id, const Vec4 &active_offset, const Vec4 &else_offset);
		inline void add_style_background_offset(Widget *w, int closet_id, const Vec4 &base_offset, const Vec2 &minus)
		{
			add_style_background_offset(w, closet_id, base_offset - Vec4(minus.x, minus.y, minus.x, minus.y), base_offset);
		}
		FLAME_UI_EXPORTS void add_style_background_color(Widget *w, int closet_id, const Bvec4 &normal_col, const Bvec4 &hovering_col, const Bvec4 &active_col);
		inline void add_style_background_color(Widget *w, int closet_id, const Vec3 &hsv)
		{
			add_style_background_color(w, closet_id, HSV(hsv.x, hsv.y, hsv.z * 0.9f, 0.9f), HSV(hsv.x, hsv.y, hsv.z, 1.f), HSV(hsv.x, hsv.y, hsv.z * 0.95f, 1.f));
		}
		FLAME_UI_EXPORTS void add_style_text_color(Widget *w, int closet_id, const Bvec4 &normal_col, const Bvec4 &else_col);
	}
}
