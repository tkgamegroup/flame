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

#include "share.h"
#include "tag.h"
#include "pic.h"
#include "tags_list.h"
#include "grid.h"
#include "detail.h"

#include <flame/img.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/instance.h>
#include <flame/UI/canvas.h>
#include <flame/UI/widgets/text.h>

#include <fstream>

using namespace flame;

int main(int argc, char **args)
{
	Ivec2 res(1280, 720);

	app = Application::create();
	auto w = Window::create(app, "Media Browser", res, WindowFrame | WindowResizable);
	w->set_maximized(true);

	d = graphics::Device::create(true);

	auto sc = graphics::Swapchain::create(d, w);

	auto image_avalible = graphics::Semaphore::create(d);
	auto ui_finished = graphics::Semaphore::create(d);

	ui = UI::Instance::create(d, sc, graphics::SampleCount_8);
	auto canvas = UI::Canvas::create(d);

	load_tags();
	load_pics();

	create_tags_list();
	create_grid_widgets();
	create_detail_widgets();

	srand(time(0));

	auto t_fps = new UI::Text(ui);
	t_fps->align = UI::AlignFloatRightBottomNoPadding;
	ui->root()->add_widget(-1, t_fps);

	app->run([&](){
		if (!w->minimized)
		{
			auto index = sc->acquire_image(image_avalible);

			ui->begin(app->elapsed_time);
			ui->end(canvas);
			canvas->record_cb(index);

			d->gq->submit(canvas->get_cb(), image_avalible, ui_finished);
			d->gq->wait_idle();
			d->gq->present(index, sc, ui_finished);

			static wchar_t buf[16];
			swprintf(buf, L"FPS:%lld", app->fps);
			t_fps->set_text_and_size(buf);
		}
	});

	return 0;
}
