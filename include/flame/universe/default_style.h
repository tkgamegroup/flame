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

namespace flame
{
	struct DefaultStyle
	{
		Vec4c text_color_normal;
		Vec4c text_color_hovering_or_active;
		Vec4c window_color;
		Vec4c frame_color_normal;
		Vec4c frame_color_hovering;
		Vec4c frame_color_active;
		Vec4c button_color_normal;
		Vec4c button_color_hovering;
		Vec4c button_color_active;
		Vec4c header_color_normal;
		Vec4c header_color_hovering;
		Vec4c header_color_active;
		float sdf_scale;

		FLAME_UNIVERSE_EXPORTS DefaultStyle();
		FLAME_UNIVERSE_EXPORTS void set_to_light();
		FLAME_UNIVERSE_EXPORTS void set_to_dark();
	};

	extern DefaultStyle default_style;
}
