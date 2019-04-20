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

#include "share.h"
#include "title_frame.h"

#include <flame/time.h>
#include <flame/math.h>
#include <flame/window.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/semaphore.h>
#include <flame/graphics/queue.h>
#include <flame/UI/canvas.h>
#include <flame/UI/widgets/text.h>

using namespace flame;

int main(int argc, char **args)
{
	Ivec2 res(500, 600);

	auto app = Application::create();
	auto w = Window::create(app, "Tetris", res, WindowFrame);

	d = graphics::Device::create(true);

	auto sc = graphics::Swapchain::create(d, w);

	ui = UI::Instance::create(d, sc, graphics::SampleCount_8);
	auto canvas = UI::Canvas::create(d);
	canvas->clear_values->set(0, Bvec4(200, 200, 200, 0));

	auto image_avalible = graphics::Semaphore::create(d);
	auto render_finished = graphics::Semaphore::create(d);
	auto ui_finished = graphics::Semaphore::create(d);

	snd_d = sound::Device::create();
	snd_c = sound::Context::create(snd_d);
	snd_c->make_current();

	snd_buf_select = sound::Buffer::create_from_file(L"sound/select.wav");
	snd_buf_ok = sound::Buffer::create_from_file(L"sound/ok.wav");
	snd_buf_back = sound::Buffer::create_from_file(L"sound/back.wav");
	snd_buf_mode = sound::Buffer::create_from_file(L"sound/mode.wav");
	snd_buf_success = sound::Buffer::create_from_file(L"sound/success.wav");
	snd_src_select = sound::Source::create(snd_buf_select);
	snd_src_ok = sound::Source::create(snd_buf_ok);
	snd_src_back = sound::Source::create(snd_buf_back);
	snd_src_mode = sound::Source::create(snd_buf_mode);
	snd_src_success = sound::Source::create(snd_buf_success);

	create_title_frame();

	auto t_fps = new UI::Text(ui);
	t_fps->align = UI::AlignFloatRightBottomNoPadding;
	ui->root()->add_widget(-1, t_fps);

	app->run([&](){
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
	});

	return 0;
}
