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

#include <flame/universe/universe.h>

namespace flame
{
	struct Element;
	typedef Element* ElementPtr;

	struct Style;
	typedef Style* StylePtr;

	FLAME_PACKAGE_BEGIN_3(StyleParm, StylePtr, thiz, p, ElementPtr, e, p, int, out_active, i1)
	FLAME_PACKAGE_END_3

	struct Style : R
	{
		int closet_id$;
		int level$;
		Function<StyleParm> f$;

		Style() 
		{
		}

		Style(int closet_id, int level, const Function<StyleParm> &f) :
			closet_id$(closet_id),
			level$(level),
			f$(f)
		{
		}

		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> background_offset(const Vec4& active_offset, const Vec4& else_offset);
		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> background_color(const Bvec4& normal_col, const Bvec4& hovering_col, const Bvec4& active_col);
		FLAME_UNIVERSE_EXPORTS static Function<StyleParm> text_color(const Bvec4& normal_col, const Bvec4& else_col);
	};
}
