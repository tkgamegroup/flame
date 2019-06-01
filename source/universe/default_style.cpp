// MIT License
// 
// Copyright (c) 2019 wjs
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

#include <flame/math.h>
#include <flame/universe/universe.h>
#include <flame/universe/default_style.h>

namespace flame
{
	DefaultStyle::DefaultStyle()
	{
		set_to_dark();
	}

	void DefaultStyle::set_to_light()
	{
		text_color_normal = Vec4c(0, 0, 0, 255);
		text_color_hovering_or_active = Vec4c(255, 255, 255, 255);
		window_color = Colorf(0.94f, 0.94f, 0.94f, 1.00f);
		frame_color_normal = Colorf(1.00f, 1.00f, 1.00f, 1.00f);
		frame_color_hovering = HSV(52.f, 0.73f, 0.97f, 0.40f);
		frame_color_active = HSV(52.f, 0.73f, 0.97f, 0.67f);
		button_color_normal = HSV(52.f, 0.73f, 0.97f, 0.40f);
		button_color_hovering = HSV(52.f, 0.73f, 0.97f, 1.00f);
		button_color_active = HSV(45.f, 0.73f, 0.97f, 1.00f);
		header_color_normal = HSV(52.f, 0.73f, 0.97f, 0.31f);
		header_color_hovering = HSV(52.f, 0.73f, 0.97f, 0.80f);
		header_color_active = HSV(52.f, 0.73f, 0.97f, 1.00f);
		sdf_scale = 1.f;
	}

	void DefaultStyle::set_to_dark()
	{
		text_color_normal = Vec4c(255, 255, 255, 255);
		text_color_hovering_or_active = Vec4c(180, 180, 180, 255);
		window_color = Colorf(0.06f, 0.06f, 0.06f, 0.94f);
		frame_color_normal = HSV(55.f, 0.67f, 0.47f, 0.54f);
		frame_color_hovering = HSV(52.f, 0.73f, 0.97f, 0.40f);
		frame_color_active = HSV(52.f, 0.73f, 0.97f, 0.67f);
		button_color_normal = HSV(52.f, 0.73f, 0.97f, 0.40f);
		button_color_hovering = HSV(52.f, 0.73f, 0.97f, 1.00f);
		button_color_active = HSV(49.f, 0.93f, 0.97f, 1.00f);
		header_color_normal = HSV(52.f, 0.73f, 0.97f, 0.31f);
		header_color_hovering = HSV(52.f, 0.73f, 0.97f, 0.80f);
		header_color_active = HSV(52.f, 0.73f, 0.97f, 1.00f);
		sdf_scale = 1.f;
	}

	DefaultStyle default_style;
}
