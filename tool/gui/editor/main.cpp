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

#include <flame/basic_app.h>

#include <flame/universe/element.h>
#include <flame/universe/ui.h>

using namespace flame;
using namespace graphics;

struct App : BasicApp
{
	Canvas* canvas;
	Font* font_msyh;
	Font* font_awesome;
	FontAtlas* font_atlas;
	int font_atlas_index;

	UI* ui;
	wText* t_fps;

	virtual void on_create() override
	{
		Canvas::initialize(d, sc);
		canvas = Canvas::create(sc);

		font_msyh = Font::create(L"c:/windows/fonts/msyh.ttc", 16);
		font_awesome = Font::create(L"../asset/font_awesome.ttf", 16);
		font_atlas = FontAtlas::create(d, 16, false, { font_msyh, font_awesome });
		font_atlas_index = canvas->add_font_atlas(font_atlas);

		ui = UI::create(canvas, w);
		auto root = ui->root();

		t_fps = Element::createT<wText>(ui, font_atlas_index);
		t_fps->align$ = AlignLeftBottom;
		root->add_child(t_fps, 1);

		auto layout = Element::createT<wLayout>(ui, LayoutHorizontal);

		auto image1 = Element::createT<wImage>(ui);
		image1->size$ = Vec2(250.f);
		image1->id() = 0;
		image1->align$ = AlignLittleEnd;

		auto image2 = Element::createT<wImage>(ui);
		image2->size$ = Vec2(250.f);
		image2->id() = 0;
		image2->align$ = AlignLittleEnd;

		auto splitter = Element::createT<wSplitter>(ui, 0, image1, image2);
		root->add_child(splitter);

		layout->add_child(image1);
		layout->add_child(splitter);
		layout->add_child(image2);
		root->add_child(layout);
	}

	virtual void do_run() override
	{
		sc->acquire_image(image_avalible);

		t_fps->text$ = L"FPS:" + std::to_wstring(app->fps);
		t_fps->set_size_auto();

		ui->step(app->elapsed_time);

		canvas->record_cb();

		d->gq->submit(canvas->get_cb(), image_avalible, render_finished);
		d->gq->wait_idle();

		d->gq->present(sc, render_finished);
	}
}app;

int main()
{
	app.create("Editor", Ivec2(1280, 720), WindowFrame);
	app.run();
}
