#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;

	graphics::Canvas* canvas;

	FontAtlas* font_atlas;

	void on_resize()
	{
		std::vector<Imageview*> vs(sc->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = sc->get_image(i)->get_default_view();

		canvas->set_target(vs.size(), vs.data());

		for (auto cb : cbs)
			cb->release();
		cbs.resize(vs.size());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i] = Commandbuffer::create(d->get_graphics_commandpool());
	}

	void run()
	{
		if (!cbs.empty())
		{
			sc->acquire_image();

			auto img_idx = sc->get_image_index();
			auto cb = cbs[img_idx];

			canvas->prepare();

			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(50.f, 20.f));
				points.push_back(Vec2f(20.f, 70.f));
				points.push_back(Vec2f(90.f, 80.f));
				canvas->fill(points.size(), points.data(), Vec4c(255), true);
			}
			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(20.f, 200.f));
				points.push_back(Vec2f(10.f, 180.f));
				points.push_back(Vec2f(110.f, 120.f));
				points.push_back(Vec2f(100.f, 100.f));
				canvas->stroke(points.size(), points.data(), Vec4c(255), 0.5f, true);
			}
			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(40.f, 200.f));
				points.push_back(Vec2f(30.f, 180.f));
				points.push_back(Vec2f(130.f, 120.f));
				points.push_back(Vec2f(120.f, 100.f));
				canvas->stroke(points.size(), points.data(), Vec4c(255), 1.f, true);
			}
			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(60.f, 200.f));
				points.push_back(Vec2f(50.f, 180.f));
				points.push_back(Vec2f(150.f, 120.f));
				points.push_back(Vec2f(140.f, 100.f));
				canvas->stroke(points.size(), points.data(), Vec4c(255), 2.f, true);
			}
			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(80.f, 200.f));
				points.push_back(Vec2f(70.f, 180.f));
				points.push_back(Vec2f(170.f, 120.f));
				points.push_back(Vec2f(160.f, 100.f));
				canvas->stroke(points.size(), points.data(), Vec4c(255), 3.f, true);
			}
			{
				std::vector<Vec2f> points;
				points.push_back(Vec2f(100.f, 200.f));
				points.push_back(Vec2f(90.f, 180.f));
				points.push_back(Vec2f(190.f, 120.f));
				points.push_back(Vec2f(180.f, 100.f));
				canvas->stroke(points.size(), points.data(), Vec4c(255), 4.f, true);
			}
			canvas->add_text(font_atlas, L"Hello World  ", -1, 14, Vec2f(5, 0), Vec4c(162, 21, 21, 255));

			canvas->record(cb, img_idx);

			fence->wait();

			if (!cbs.empty())
			{
				auto q = d->get_graphics_queue();
				q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
				q->present(sc, render_finished);
			}
		}
	}

}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	set_engine_path(engine_path.c_str());

	app.w = Window::create("Graphics Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.canvas = Canvas::create(app.d);
	app.canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	app.on_resize();
	app.w->add_resize_listener([](Capture&, const Vec2u&) {
		app.on_resize();
	}, Capture());

	{
		Font* fonts[] = {
			Font::create(L"c:/windows/fonts/consola.ttf"),
			Font::create((engine_path / L"art/font_awesome.ttf").c_str())
		};
		app.font_atlas = FontAtlas::create(app.d, 2, fonts);
	}
	app.canvas->add_font(app.font_atlas);

	get_looper()->loop([](Capture&, float) {
		app.run();
	}, Capture());

	return 0;
}
