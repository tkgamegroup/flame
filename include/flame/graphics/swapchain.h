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
	struct Window;

	namespace graphics
	{
		struct Device;
		struct Image;
		struct Renderpass;
		struct Framebuffer;
		struct Semaphore;

		FLAME_GRAPHICS_EXPORTS Format get_swapchain_format();

		struct Swapchain
		{
			FLAME_GRAPHICS_EXPORTS SampleCount sample_count() const;
			FLAME_GRAPHICS_EXPORTS Window *window() const;
			FLAME_GRAPHICS_EXPORTS Image *image(int idx) const;
			FLAME_GRAPHICS_EXPORTS uint image_index() const;
			FLAME_GRAPHICS_EXPORTS Renderpass *renderpass(bool clear = false) const;
			FLAME_GRAPHICS_EXPORTS Framebuffer *framebuffer(uint index) const;

			FLAME_GRAPHICS_EXPORTS void acquire_image(Semaphore *signal_semaphore);

			FLAME_GRAPHICS_EXPORTS static Swapchain *create(Device *d, Window *w, SampleCount sc = SampleCount_1);
			FLAME_GRAPHICS_EXPORTS static void destroy(Swapchain *s);
		};
	}
}

