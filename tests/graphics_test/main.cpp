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

using namespace flame;
using namespace graphics;

struct MyApp : BasicApp
{
	//Commandbuffer* cbs[2];
	//ClearValues* cv;
	//BP* render_path;
	Canvas* canvas;

	inline virtual void on_create() override
	{
		//render_path = BP::create_from_file(L"graphics_test_renderpath.bp");
		//render_path->find_item("d.in")->set_data(d);
		//render_path->find_item("sc.in")->set_data(sc);
		//render_path->prepare();
		//render_path->update();
		//render_path->unprepare();
		//cv = (ClearValues*)render_path->find_item("cv.out")->data().v.p;
		//cbs[0]  = (Commandbuffer*)render_path->find_item("cb1.out")->data().v.p;
		//cbs[1] = (Commandbuffer*)render_path->find_item("cb2.out")->data().v.p;

		canvas = Canvas::create(d, sc);
	}

	inline virtual void do_run() override
	{
		sc->acquire_image(image_avalible);

		canvas->add_rect_filled(Vec2(0.f), Vec2(100.f), Bvec4(255, 128, 0, 255));
		canvas->record_cb();

		d->gq->submit(canvas->get_cb(), image_avalible, render_finished);
		d->gq->wait_idle();

		d->gq->present(sc, render_finished);
	}
}app;

extern "C" __declspec(dllexport) int main()
{
	Ivec2 res(1280, 720);

	app.create("Graphics Test", res, WindowFrame);
	app.run();

	return 0;
}
