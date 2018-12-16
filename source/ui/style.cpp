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
		FLAME_DATA_PACKAGE_BEGIN(StyleBackgroundOffsetData, Widget::StyleParm)
			FLAME_DATA_PACKAGE_CAPT(Vec4, active_offset, f4)
			FLAME_DATA_PACKAGE_CAPT(Vec4, else_offset, f4)
		FLAME_DATA_PACKAGE_END

		FLAME_REGISTER_FUNCTION_BEG(StyleBackgroundOffse, FLAME_GID(24726), StyleBackgroundOffsetData)
			if (p.thiz()->closet_id$ != p.closet_id())
				return;

			switch (p.thiz()->state)
			{
			case StateNormal: case StateHovering:
				if (p.thiz()->style_level <= 0)
				{
					p.thiz()->background_offset$ = p.else_offset();
					p.thiz()->style_level = 0;
				}
				break;
			case StateActive:
				if (p.thiz()->style_level <= 0)
				{
					p.thiz()->background_offset$ = p.active_offset();
					p.thiz()->style_level = 0;
				}
				break;
			}
		FLAME_REGISTER_FUNCTION_END(StyleBackgroundOffse)

		void add_style_background_offset(Widget *w, int closet_id, const Vec4 &active_offset, const Vec4 &else_offset)
		{
			w->add_style(closet_id, StyleBackgroundOffse::v, { active_offset, else_offset });
		}

		FLAME_DATA_PACKAGE_BEGIN(StyleBackgroundColorData, Widget::StyleParm)
			FLAME_DATA_PACKAGE_CAPT(Bvec4, normal_color, b4)
			FLAME_DATA_PACKAGE_CAPT(Bvec4, hovering_color, b4)
			FLAME_DATA_PACKAGE_CAPT(Bvec4, active_color, b4)
		FLAME_DATA_PACKAGE_END

		FLAME_REGISTER_FUNCTION_BEG(StyleBackgroundColor, FLAME_GID(13741), StyleBackgroundColorData)
			if (p.thiz()->closet_id$ != p.closet_id())
				return;

			switch (p.thiz()->state)
			{
			case StateNormal:
				if (p.thiz()->style_level <= 0)
				{
					p.thiz()->background_col$ = p.normal_color();
					p.thiz()->style_level = 0;
				}
				break;
			case StateHovering:
				if (p.thiz()->style_level <= 0)
				{
					p.thiz()->background_col$ = p.hovering_color();
					p.thiz()->style_level = 0;
				}
				break;
			case StateActive:
				if (p.thiz()->style_level <= 0)
				{
					p.thiz()->background_col$ = p.active_color();
					p.thiz()->style_level = 0;
				}
				break;
			}
		FLAME_REGISTER_FUNCTION_END(StyleBackgroundColor)

		void add_style_background_color(Widget *w, int closet_id, const Bvec4 &normal_col, const Bvec4 &hovering_col, const Bvec4 &active_col)
		{
			w->add_style(closet_id, StyleBackgroundColor::v, { normal_col, hovering_col, active_col });
		}

		struct StyleTextColorData : Widget::StyleParm
		{
			enum { BASE = __COUNTER__ + 1 };

			inline Bvec4 &normal_color()
			{
				return d[__COUNTER__ - BASE + Widget::StyleParm::SIZE].b4();
			}
			inline Bvec4 &else_color()
			{
				return d[__COUNTER__ - BASE + Widget::StyleParm::SIZE].b4();
			}
		};

		FLAME_REGISTER_FUNCTION_BEG(StyleTextColor, FLAME_GID(785), StyleTextColorData)
			if (p.thiz()->closet_id$ != p.closet_id())
				return;

			switch (p.thiz()->state)
			{
			case StateNormal:
				((wText*)p.thiz())->text_col() = p.normal_color();
				break;
			case StateHovering: case StateActive:
				((wText*)p.thiz())->text_col() = p.else_color();
				break;
			}
		FLAME_REGISTER_FUNCTION_END(StyleTextColor)

		void add_style_text_color(Widget *w, int closet_id, const Bvec4 &normal_col, const Bvec4 &else_col)
		{
			w->add_style(closet_id, StyleTextColor::v, { normal_col, else_col });
		}
	}
}

