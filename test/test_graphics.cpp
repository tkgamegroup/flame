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

using namespace flame;
using namespace graphics;

struct App : BasicApp
{
	VoidPtrs cbs;
	BP* render_path;
	//Canvas* canvas;
	//Font* font;
	//int font_index;

	float last_time;

	virtual void on_create() override
	{
		render_path = BP::create_from_file(L"../renderpath/test/renderpath.bp");
		render_path->find_input("d.in")->set_data(&d);
		render_path->find_input("sc.in")->set_data(&sc);
		render_path->initialize();
		render_path->update();
		//render_path->finish();
		memcpy(&cbs, render_path->find_output("cb.out")->data(), sizeof(VoidPtrs));

		//Canvas::initialize(d, sc);
		//canvas = Canvas::create(sc);

		//font = Font::create(d, L"c:/windows/fonts/msyh.ttc", 32, true);
		//font_index = canvas->add_font(font);

		last_time = 0.f;
	}

	virtual void do_run() override
	{
		auto idx = frame % FLAME_ARRAYSIZE(fences);

		sc->acquire_image(image_avalible);

		fences[idx]->wait();

		d->gq->submit((graphics::Commandbuffer*)cbs.v[sc->image_index()], image_avalible, render_finished, fences[idx]);

		d->gq->present(sc, render_finished);

		frame++;

		last_time += app->elapsed_time;
		if (last_time > 1.f)
		{
			last_time -= 1.f;
			printf("%d\n", (int)app->fps);
		}
	}
}app;

int main(int argc, char** args)
{
	Ivec2 res(1280, 720);

	app.create("Graphics Test", res, WindowFrame, graphics::SampleCount_1);
	app.run();

	return 0;
}
