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

#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/canvas.h>
#include <flame/graphics/font.h>

namespace flame
{
	struct BasicApp
	{
		Application *app;
		Window *w;
		graphics::Device *d;
		graphics::Swapchain *sc;
		graphics::Semaphore *image_avalible;
		graphics::Semaphore *render_finished;

		virtual void on_create() = 0;

		inline void create(const char *title, const Ivec2 &res, int style)
		{
			typeinfo_load(L"typeinfo.xml");
			app = Application::create();
			w = Window::create(app, title, res, style);
			d = graphics::Device::/*get_shared*/create(true);
			sc = graphics::Swapchain::create(d, w, graphics::SampleCount_8);
			image_avalible = graphics::Semaphore::create(d);
			render_finished = graphics::Semaphore::create(d);

			on_create();
		}

		virtual void do_run() = 0;

		inline void run()
		{
			app->run(Function<>([](Package &p) {
				auto thiz = (BasicApp*)p.d[0].p();
				thiz->do_run();
			}, { this }));
		}
	};
}
