#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;

	graphics::Canvas* canvas;

	FontAtlas* font_atlas;

	void on_resize()
	{
		std::vector<Imageview*> vs(sc->image_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = sc->image(i)->default_view();

		canvas->set_target(vs.size(), vs.data());

		cbs.resize(vs.size());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i] = Commandbuffer::create(d->gcp);
	}

	void run()
	{
		if (!cbs.empty())
			sc->acquire_image();

		auto img_idx = sc->image_index();
		auto cb = cbs[img_idx];

		canvas->prepare();
		{
			std::vector<Vec2f> points;
			path_rect(points, Vec2f(100.f), Vec2f(200.f));
			canvas->fill(points.size(), points.data(), Vec4c(255));
		}
		canvas->add_text(font_atlas, L"Hello World  ", nullptr, 14, Vec2f(5, 0), Vec4c(162, 21, 21, 255));
		canvas->add_text(font_atlas, L"中文", nullptr, 14, Vec2f(100, 100), Vec4c(0, 0, 0, 255));
		canvas->record(cb, img_idx);

		fence->wait();

		if (!cbs.empty())
		{
			d->gq->submit(1, &cb, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}

}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	set_engine_path(engine_path.c_str());

	app.w = SysWindow::create("Graphics Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.canvas = Canvas::create(app.d);
	app.canvas->clear_color = Vec4f(0.4f, 0.4f, 0.4f, 1.f);
	app.on_resize();
	app.w->resize_listeners.add([](void*, const Vec2u&) {
		app.on_resize();
		return true;
	}, Mail());

	{
		auto font_awesome_path = engine_path / L"art/font_awesome.ttf";
		const wchar_t* fonts[] = {
			L"c:/windows/fonts/consola.ttf",
			font_awesome_path.c_str(),
		};
		app.font_atlas = FontAtlas::create(app.d, 2, fonts);
	}
	app.canvas->add_font(app.font_atlas);

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
