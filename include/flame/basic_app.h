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

#include <flame/serialize.h>
#include <flame/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/ui/instance.h>
#include <flame/ui/canvas.h>
#include <flame/ui/widget.h>
#ifdef HAS_3D
#include <flame/3d/scene.h>
#endif

namespace flame
{
	struct BasicApp;
	typedef BasicApp* BasicAppPtr;

	struct BasicApp
	{
		Application *app;
		Window *w;
		graphics::Device *d;
		graphics::Swapchain *sc;
		graphics::Semaphore *image_avalible;
#ifdef HAS_3D
		graphics::Semaphore *scene_finished;
#endif
		graphics::Semaphore *ui_finished;
		ui::SwapchainData *sd;
		ui::Instance *ui_ins;
		ui::Canvas *canvas;
		ui::wText *t_fps;
#ifdef HAS_3D
		_3d::Scene *scene_3d;
#endif

		inline void create(const char *title, const Ivec2 &res, int style)
		{
			typeinfo_load(L"typeinfo.xml");
			app = Application::create();
			w = Window::create(app, title, res, style);
			d = graphics::Device::get_shared();
			sc = graphics::Swapchain::create(d, w);
			image_avalible = graphics::Semaphore::create(d);
#ifdef HAS_3D
				scene_finished = graphics::Semaphore::create(d);
#endif
			ui_finished = graphics::Semaphore::create(d);
			sd = ui::SwapchainData::create(sc);
			ui_ins = ui::Instance::create(w);
			canvas = ui::Canvas::create(sd);

			t_fps = ui::Widget::createT<ui::wText>(ui_ins);
			t_fps->align$ = ui::AlignRightBottomNoPadding;
			t_fps->text_col() = Bvec4(0, 0, 0, 255);
			ui_ins->root()->add_child(t_fps, 1);

#ifdef HAS_3D
			scene_3d = _3d::Scene::create(res);
#endif
		}

		static void do_run(const ParmPackage &p)
		{
			auto &thiz = (BasicAppPtr&)p.d[0].p();

			auto index = thiz->sc->acquire_image(thiz->image_avalible);

#ifdef HAS_3D
			thiz->scene_3d->begin(thiz->app->elapsed_time);
#endif

			thiz->ui_ins->begin(thiz->app->elapsed_time);
			thiz->ui_ins->end(thiz->canvas);

#ifdef HAS_3D
			thiz->scene_3d->end();

			thiz->scene_3d->record_cb();
#endif
			thiz->canvas->record_cb(index);

#ifdef HAS_3D
			thiz->d->gq->submit(thiz->scene_3d->get_cb(), thiz->image_avalible, thiz->scene_finished);
			thiz->d->gq->submit(thiz->canvas->get_cb(), thiz->scene_finished, thiz->ui_finished);
#else
			thiz->d->gq->submit(thiz->canvas->get_cb(), thiz->image_avalible, thiz->ui_finished);
#endif
			thiz->d->gq->wait_idle();
			thiz->d->gq->present(index, thiz->sc, thiz->ui_finished);

			static wchar_t buf[16];
			swprintf(buf, L"FPS:%lld", thiz->app->fps);
			thiz->t_fps->text() = buf;
			thiz->t_fps->set_size_auto();
		}

		inline void run()
		{
			app->run(do_run, { this });
		}
	};
}
