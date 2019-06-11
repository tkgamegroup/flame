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
	struct Bitmap;

	namespace graphics
	{
		struct Device;

		struct Image
		{
			Format$ format;
			Vec2u size;
			uint level;
			uint layer;
			SampleCount$ sample_count;

			uint channel_, bpp_;
			uint pitch_;
			uint data_size_;

			FLAME_GRAPHICS_EXPORTS static Format$ find_format(uint channel, uint bpp);

			// a layout change from Undefined to ShaderReadOnly, and a clear
			FLAME_GRAPHICS_EXPORTS void init(const Vec4c& col);

			// accepted formats for get/set pixels: Format_R8G8B8A8_UNORM, Format_R16G16B16A16_UNORM
			FLAME_GRAPHICS_EXPORTS void get_pixels(uint x, uint y, int cx/* -1 means whole */, int cy/* -1 means whole */, void* dst);
			FLAME_GRAPHICS_EXPORTS void set_pixels(uint x, uint y, int cx/* -1 means whole */, int cy/* -1 means whole */, const void* src);

			FLAME_GRAPHICS_EXPORTS void save_png(const wchar_t* filename);

			FLAME_GRAPHICS_EXPORTS static Image* create(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, SampleCount$ sample_count, ImageUsage$ usage, void* data = nullptr);
			// default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create_from_bitmap(Device* d, Bitmap* bmp, ImageUsage$ extra_usage = ImageUsage$(0));
			// default usage: ShaderSampled, TransferDst
			FLAME_GRAPHICS_EXPORTS static Image* create_from_file(Device* d, const wchar_t* filename, ImageUsage$ extra_usage = ImageUsage$(0));
			FLAME_GRAPHICS_EXPORTS static Image* create_from_native(Device* d, Format$ format, const Vec2u& size, uint level, uint layer, void* native);

			FLAME_GRAPHICS_EXPORTS static void destroy(Image* i);
		};

		struct Imageview
		{
			ImageviewType$ type;
			uint base_level;
			uint level_count;
			uint base_layer;
			uint layer_count;
			Swizzle$ swizzle_r;
			Swizzle$ swizzle_g;
			Swizzle$ swizzle_b;
			Swizzle$ swizzle_a;

			FLAME_GRAPHICS_EXPORTS Image* image() const;

			FLAME_GRAPHICS_EXPORTS static Imageview* create(Image* i, ImageviewType$ type = Imageview2D, uint base_level = 0, uint level_count = 1, uint base_layer = 0, uint layer_count = 1, 
				Swizzle$ swizzle_r = SwizzleIdentity, Swizzle$ swizzle_g = SwizzleIdentity, Swizzle$ swizzle_b = SwizzleIdentity, Swizzle$ swizzle_a = SwizzleIdentity);
			FLAME_GRAPHICS_EXPORTS static void destroy(Imageview* v);
		};

		struct Sampler
		{
			FLAME_GRAPHICS_EXPORTS static Sampler* create(Device* d, Filter mag_filter, Filter min_filter, bool unnormalized_coordinates);
			FLAME_GRAPHICS_EXPORTS static void destroy(Sampler* s);
		};
	}
}

