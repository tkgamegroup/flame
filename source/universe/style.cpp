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

#include <flame/foundation/foundation.h>
#include <flame/universe/element.h>

using namespace flame;

namespace flame
{
	FLAME_PACKAGE_BEGIN_2(StyleBackgroundOffsetData, Vec4, active_offset, f4, Vec4, else_offset, f4)
	FLAME_PACKAGE_END_2

	void style_background_offset(StyleParm& p)
	{
		auto e = p.e();
		auto& c = p.get_capture<StyleBackgroundOffsetData>();
		switch (e->state)
		{
		case StateNormal: case StateHovering:
			e->background_offset$ = c.else_offset();
			break;
		case StateActive:
			e->background_offset$ = c.active_offset();
			break;
		}
		p.out_active() = 1;
	}

	Function<StyleParm> Style::background_offset(const Vec4& active_offset, const Vec4& else_offset)
	{
		return Function<StyleParm>(style_background_offset, { active_offset, else_offset });
	}

	FLAME_PACKAGE_BEGIN_3(StyleBackgroundColorData, Bvec4, normal_color, b4, Bvec4, hovering_color, b4, Bvec4, active_color, b4)
	FLAME_PACKAGE_END_3

	void style_background_color(StyleParm& p)
	{
		auto e = p.e();
		auto& c = p.get_capture<StyleBackgroundColorData>();
		switch (e->state)
		{
		case StateNormal:
			e->background_col$ = c.normal_color();
			break;
		case StateHovering:
			e->background_col$ = c.hovering_color();
			break;
		case StateActive:
			e->background_col$ = c.active_color();
			break;
		}
		p.out_active() = 1;
	}

	Function<StyleParm> Style::background_color(const Bvec4& normal_col, const Bvec4& hovering_col, const Bvec4& active_col)
	{
		return Function<StyleParm>(style_background_color, { normal_col, hovering_col, active_col });
	}

	FLAME_PACKAGE_BEGIN_2(StyleTextColorData, Bvec4, normal_color, b4, Bvec4, else_color, b4)
	FLAME_PACKAGE_END_2

	void style_text_color(StyleParm& p)
	{
		auto e = (wTextPtr)p.e();
		auto& c = p.get_capture<StyleTextColorData>();
		switch (e->state)
		{
		case StateNormal:
			e->text_col() = c.normal_color();
			break;
		case StateHovering: case StateActive:
			e->text_col() = c.else_color();
			break;
		}
		p.out_active() = 1;
	}

	Function<StyleParm> Style::text_color(const Bvec4& normal_col, const Bvec4& else_col)
	{
		return Function<StyleParm>(style_text_color, { normal_col, else_col });
	}
}

