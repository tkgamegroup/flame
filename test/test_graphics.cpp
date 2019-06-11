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
	std::wstring rp_filename;
	std::fs::file_time_type rp_lwt;
	BP* rp;
	Array<void*> cbs;

	//Canvas* canvas;
	//Font* font;
	//int font_index;

	float last_time;

	virtual void on_create() override
	{
		rp_filename = L"../renderpath/canvas/renderpath.bp";
		rp = nullptr;
		memset(&cbs, 0, sizeof(cbs));

		//Canvas::initialize(d, sc);
		//canvas = Canvas::create(sc);

		//font = Font::create(d, L"c:/windows/fonts/msyh.ttc", 32, true);
		//font_index = canvas->add_font(font);

		last_time = 0.f;
	}

	virtual void do_run() override
	{
		auto idx = frame % FLAME_ARRAYSIZE(fences);

		if (cbs.v)
			sc->acquire_image(image_avalible);

		if (fences[idx].second > 0)
		{
			fences[idx].first->wait();
			fences[idx].second = 0;
		}

		if (cbs.v)
		{
			d->gq->submit((graphics::Commandbuffer*)cbs.v[sc->image_index()], image_avalible, render_finished, fences[idx].first);
			fences[idx].second = 1;

			d->gq->present(sc, render_finished);
		}

		frame++;

		last_time += app->elapsed_time;
		if (last_time > 1.f)
		{
			last_time -= 1.f;
			printf("%d\n", (int)app->fps);

			if (std::fs::exists(rp_filename))
			{
				if (!rp || rp_lwt < std::fs::last_write_time(rp_filename))
				{
					typeinfo_clear();
					typeinfo_check_update();
					typeinfo_load(L"flame_foundation.typeinfo");
					typeinfo_load(L"flame_graphics.typeinfo");

					rp_lwt = std::fs::last_write_time(rp_filename);

					rp = BP::create_from_file(rp_filename);
					rp->find_input("d.in")->set_data(&d);
					rp->find_input("sc.in")->set_data(&sc);
					rp->initialize();
					rp->update();
					memcpy(&cbs, rp->find_output("cbs.out")->data(), sizeof(Array<void*>));
				}
			}
			else
			{
				if (rp)
				{
					rp->finish();
					BP::destroy(rp);
				}

				rp = nullptr;
				memset(&cbs, 0, sizeof(cbs));
			}
		}
	}
}app;

int main(int argc, char** args)
{
	app.create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.run();

	return 0;
}
