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

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Image;

		FLAME_GRAPHICS_EXPORTS void get_latin_code_range(wchar_t &out_begin, wchar_t &out_end);

		struct Glyph
		{
			Ivec2 off;
			Ivec2 size;
			Vec2 uv0, uv1;
			int advance;
		};

		struct Font
		{
			FLAME_GRAPHICS_EXPORTS int pixel_height() const;
			FLAME_GRAPHICS_EXPORTS int max_width() const;
			FLAME_GRAPHICS_EXPORTS int ascender() const;
				
			FLAME_GRAPHICS_EXPORTS const Glyph *get_glyph(wchar_t unicode);
			FLAME_GRAPHICS_EXPORTS int get_text_width(const wchar_t *text_beg, const wchar_t *text_end = nullptr);

			FLAME_GRAPHICS_EXPORTS Image *get_atlas() const;

			FLAME_GRAPHICS_EXPORTS static Font *create(Device *d, const wchar_t *filename, int pixel_height, bool sdf = false);
			FLAME_GRAPHICS_EXPORTS static void destroy(Font *f);
		};
	}
}
