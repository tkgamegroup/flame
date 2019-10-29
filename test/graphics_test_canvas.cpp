#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	BP* canvas_bp;
	Canvas* canvas;

	FontAtlas* font_atlas1;
	FontAtlas* font_atlas2;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_delay_events();

		if (sc)
		{
			std::vector<Vec2f> points;
			path_rect(points, Vec2f(100.f), Vec2f(200.f));
			canvas->fill(points, Vec4c(255));
			canvas->add_text(font_atlas1, Vec2f(5, 0), Vec4c(162, 21, 21, 255), L"Hello World  ");
			canvas->add_text(font_atlas2, Vec2f(100, 100), Vec4c(0, 0, 0, 255), L"中文", 0.375f);
		}
		canvas_bp->update();

		if (sc)
		{
			d->gq->submit({ cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}

		scr->signal = false;
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	app.canvas_bp = BP::create_from_file(L"../renderpath/canvas_make_cmd/bp", true);
	if (!app.canvas_bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = Window::create("Graphics Test", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app.canvas_bp->graphics_device = app.d;
	app.scr->link_bp(app.canvas_bp, app.cbs);
	app.canvas_bp->update();
	app.canvas = (Canvas*)app.canvas_bp->find_output("*.make_cmd.canvas")->data_p();
	app.canvas->set_clear_color(Vec4c(100, 100, 100, 255));

	auto font_msyh = Font::create(L"c:/windows/fonts/consola.ttf", 14);
	auto font_awesome = Font::create(L"../asset/font_awesome.ttf", 14);
	app.font_atlas1 = FontAtlas::create(app.d, FontDrawPixel, { font_msyh, font_awesome });
	app.font_atlas2 = FontAtlas::create(app.d, FontDrawSdf, { font_msyh });
	app.font_atlas1->index = app.canvas->set_image(-1, app.font_atlas1->imageview());
	app.font_atlas2->index = app.canvas->set_image(-1, app.font_atlas2->imageview());

	looper().loop([](void* c) {
		auto app = *(App**)c;
		app->run();
	}, new_mail_p(&app));
}
