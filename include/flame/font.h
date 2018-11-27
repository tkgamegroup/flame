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

#ifdef FLAME_WINDOWS
#ifdef FLAME_FONT_MODULE
#define FLAME_FONT_EXPORTS __declspec(dllexport)
#else
#define FLAME_FONT_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_FONT_EXPORTS
#endif

#include <flame/math.h>

#include <vector>

namespace flame
{
	struct Bitmap;

	struct CharRange
	{
		wchar_t code_begin;
		wchar_t code_end;
		bool sdf;

		CharRange() :
			code_begin(0),
			code_end(0),
			sdf(false)
		{
		}

		CharRange(wchar_t _code_begin, int _code_end, bool _sdf) :
			code_begin(_code_begin),
			code_end(_code_end),
			sdf(_sdf)
		{
		}
	};

	FLAME_FONT_EXPORTS void get_default_char_range(wchar_t &out_code_begin, wchar_t &out_code_end);

	struct Glyph
	{
		wchar_t unicode;

		Ivec2 off;
		Ivec2 size;
		Ivec2 img_off;
		Ivec2 sdf_img_off;
		Vec2 uv0, uv1;
		Vec2 uv0_sdf, uv1_sdf;
		int advance;
		int ascent;

		Glyph(wchar_t uc) :
			unicode(uc),
			off(0),
			size(0),
			img_off(0),
			sdf_img_off(0),
			uv0(0.f),
			uv1(0.f),
			uv0_sdf(0.f),
			uv1_sdf(0.f),
			advance(0),
			ascent(0)
		{
		}
	};

	struct FontDescription
	{
		const wchar_t *filename;
		std::vector<CharRange> ranges;
	};

	struct FontAtlas
	{
		int pixel_height;

		FLAME_FONT_EXPORTS const Glyph &get_glyph(wchar_t unicode);

		FLAME_FONT_EXPORTS int get_text_width(const wchar_t *text);

		FLAME_FONT_EXPORTS Bitmap *get_stroke_image() const;
		FLAME_FONT_EXPORTS Bitmap *get_sdf_image() const;

		FLAME_FONT_EXPORTS void save(const wchar_t *filename) const;

		FLAME_FONT_EXPORTS static FontAtlas *create(const std::vector<FontDescription> &descs, int pixel_height, float sdf_scale = -1.f);
		FLAME_FONT_EXPORTS static FontAtlas *create_from_file(const wchar_t *filename);
		FLAME_FONT_EXPORTS static void destroy(FontAtlas *f);
	};
}
