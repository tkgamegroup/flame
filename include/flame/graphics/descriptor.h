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

#include <vector>

namespace flame
{
	namespace graphics
	{
		struct Device;
		struct Buffer;
		struct Imageview;
		struct Sampler;

		struct Descriptorpool
		{
			FLAME_GRAPHICS_EXPORTS static Descriptorpool *create(Device *d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorpool *p);
		};

		struct Descriptorsetlayout
		{
			struct Binding
			{
				uint binding;
				ShaderResourceType type;
				uint count;
			};

			FLAME_GRAPHICS_EXPORTS static Descriptorsetlayout *create(Device *d, const std::vector<Binding> &bindings);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorsetlayout *l);
		};

		struct Descriptorset
		{
			FLAME_GRAPHICS_EXPORTS void set_uniformbuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_storagebuffer(uint binding, uint index, Buffer *b, uint offset = 0, uint range = 0);
			FLAME_GRAPHICS_EXPORTS void set_imageview(uint binding, uint index, Imageview *v, Sampler *sampler);
			FLAME_GRAPHICS_EXPORTS void set_storageimage(uint binding, uint index, Imageview *v);

			FLAME_GRAPHICS_EXPORTS static Descriptorset *create(Descriptorpool *p, Descriptorsetlayout *l);
			FLAME_GRAPHICS_EXPORTS static void destroy(Descriptorset *s);
		};
	}
}

