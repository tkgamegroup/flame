#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	Swapchain* sc;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;

	FontAtlas* font_atlas1;
	FontAtlas* font_atlas2;
	Canvas* canvas;

	void run()
	{
		sc->acquire_image();
		fence->wait();

		canvas->add_text(font_atlas1, Vec2f(5, 0), Vec4c(162, 21, 21, 255), L"Hello World  ");
		canvas->add_text(font_atlas2, Vec2f(100, 100), Vec4c(0, 0, 0, 255), L"中文", 0.375f);

		auto cb = cbs[sc->image_index()];
		canvas->record(cb);
		
		d->gq->submit(cb, sc->image_avalible(), render_finished, fence);
		d->gq->present(sc, render_finished);
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_graphics.typeinfo");

	app.w = Window::create("Graphics Test", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);

	app.canvas = Canvas::create(app.d, app.sc);
	//app.canvas->set_clear_color(Vec4c(204, 213, 240, 255));
	app.canvas->set_clear_color(Vec4c(255));

	auto font_msyh = Font::create(L"c:/windows/fonts/consola.ttf", 14);
	auto font_awesome = Font::create(L"../asset/font_awesome.ttf", 14);
	app.font_atlas1 = FontAtlas::create(app.d, FontDrawPixel, { font_msyh, font_awesome });
	app.font_atlas2 = FontAtlas::create(app.d, FontDrawSdf, { font_msyh });
	auto font_atlas_view1 = Imageview::create(app.font_atlas1->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR);
	auto font_atlas_view2 = Imageview::create(app.font_atlas2->image());
	app.font_atlas1->index = 1;
	app.font_atlas2->index = 2;
	app.canvas->set_image(app.font_atlas1->index, font_atlas_view1);
	app.canvas->set_image(app.font_atlas2->index, font_atlas_view2);

	app.cbs.resize(app.sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
