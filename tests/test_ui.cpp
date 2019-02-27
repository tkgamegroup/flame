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

#include <flame/basic_app.h>

#include <flame/universe/element.h>
#include <flame/universe/ui.h>

using namespace flame;
using namespace graphics;

struct UIApp : BasicApp
{
	Canvas* canvas;
	Font* font;
	int font_index;

	UI* ui;

	inline virtual void on_create() override
	{
		Canvas::initialize(d, sc);
		canvas = Canvas::create(sc);

		font = Font::create(d, L"c:/windows/fonts/msyh.ttc", 32, true);
		font_index = canvas->add_font(font);

		ui = UI::create(canvas, w);

		auto btn = Element::createT<wButton>(ui, font_index, L"hit me");
		ui->root()->add_child(btn);
	}

	inline virtual void do_run() override
	{
		sc->acquire_image(image_avalible);

		ui->begin(app->elapsed_time);
		ui->end();

		canvas->record_cb();

		d->gq->submit(canvas->get_cb(), image_avalible, render_finished);
		d->gq->wait_idle();

		d->gq->present(sc, render_finished);
	}
}app;

void test_ui()
{
	Ivec2 res(1280, 720);

	app.create("UI Test", res, WindowFrame);
	app.run();
}
