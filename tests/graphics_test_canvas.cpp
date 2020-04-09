#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;

	graphics::Canvas* canvas;

	FontAtlas* font_atlas;

	void draw()
	{
		std::vector<Vec2f> points;
		path_rect(points, Vec2f(100.f), Vec2f(200.f));
		canvas->fill(points.size(), points.data(), Vec4c(255));
		canvas->add_text(font_atlas, L"Hello World  ", nullptr, 14, Vec2f(5, 0), Vec4c(162, 21, 21, 255));
		canvas->add_text(font_atlas, L"中文", nullptr, 14, Vec2f(100, 100), Vec4c(0, 0, 0, 255));
	}

	void run()
	{
		if (sc->image_count())
			sc->acquire_image();

		looper().process_events();

		canvas->scene->update();

		fence->wait();

		if (sc->image_count())
		{
			d->gq->submit(1, &cbs.v[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}

}app;

int main(int argc, char** args)
{
	TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
	TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
	TypeinfoDatabase::load(L"flame_universe.dll", true, true);

	std::filesystem::path engine_path = getenv("FLAME_PATH");

	auto bp = BP::create_from_file((engine_path / L"renderpath/canvas/bp").c_str());
	if (!bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = SysWindow::create("Graphics Test", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.sc->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app.sc->link_bp(bp, &app.cbs);
	bp->update();
	app.canvas = (Canvas*)bp->find_output("make_cmd.canvas")->data_p();
	app.canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	app.canvas->set_draw([](void*) {
		app.draw();
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
