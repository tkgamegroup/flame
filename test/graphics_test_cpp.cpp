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

struct App
{
	Window* w;
	graphics::Device* d;
	graphics::Semaphore* render_finished;
	graphics::Swapchain* sc;
	graphics::Fence* fence;
	std::vector<graphics::Commandbuffer*> cbs;

	graphics::FontAtlas* font_atlas;
	graphics::Canvas* canvas;

	void run()
	{
		sc->acquire_image();
		fence->wait();

		canvas->add_text(app.font_atlas, Vec2f(100, 50), Vec4c(200, 160, 230, 255), L"Hello World");

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

	app.w = Window::create("", Vec2u(1280, 720), WindowFrame);
	app.d = graphics::Device::create(true);
	app.render_finished = graphics::Semaphore::create(app.d);
	app.sc = graphics::Swapchain::create(app.d, app.w);
	app.fence = graphics::Fence::create(app.d);

	app.canvas = graphics::Canvas::create(app.d, app.sc);

	auto font_msyh = graphics::Font::create(L"c:/windows/fonts/msyh.ttc", 16);
	app.font_atlas = graphics::FontAtlas::create(app.d, 16, false, { font_msyh });
	auto font_atlas_view = graphics::Imageview::create(app.font_atlas->image());
	app.font_atlas->index = 1;
	app.canvas->set_image(app.font_atlas->index, font_atlas_view);

	app.cbs.resize(app.sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = graphics::Commandbuffer::create(app.d->gcp);

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
