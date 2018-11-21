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
#ifdef FLAME_BITMAP_MODULE
#define FLAME_BITMAP_EXPORTS __declspec(dllexport)
#else
#define FLAME_BITMAP_EXPORTS __declspec(dllimport)
#endif
#else
#define FLAME_BITMAP_EXPORTS
#endif

#include "math.h"

namespace flame
{
	struct Bitmap
	{
		Ivec2 size;
		int channel;
		int bpp;
		bool sRGB;
		int pitch;
		unsigned char *data;
		int data_size;

		static inline int get_pitch(int line_bytes)
		{
			auto p = line_bytes;
			if (p % 4 == 0)
				return p;
			return p + 4 - p % 4;
		}

		static inline int get_pitch(int cx, int bpp)
		{
			return get_pitch(cx * (bpp / 8));
		}

		inline void calc_pitch()
		{
			pitch = get_pitch(size.x, bpp);
		}

		FLAME_BITMAP_EXPORTS void add_alpha_channel();
		FLAME_BITMAP_EXPORTS void swap_channel(int ch1, int ch2);
		FLAME_BITMAP_EXPORTS void copy_to(Bitmap *dst, const Ivec2 &src_off, const Ivec2 &cpy_size, const Ivec2 &dst_off);

		FLAME_BITMAP_EXPORTS void save(const wchar_t *filename);

		FLAME_BITMAP_EXPORTS static Bitmap *create(const Ivec2 &size, int channel, int bpp, unsigned char *data = nullptr, bool data_owner = false);
		FLAME_BITMAP_EXPORTS static Bitmap *create_from_file(const wchar_t *filename);
		FLAME_BITMAP_EXPORTS static Bitmap *create_from_gif(const wchar_t *filename);
		FLAME_BITMAP_EXPORTS static void destroy(Bitmap *b);
	};
}
