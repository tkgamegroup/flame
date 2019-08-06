#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	std::vector<Fence*> fences;
	std::vector<void*> cbs;
	BP* bp;
	BP::Slot* images_slot;

	void run()
	{
		auto sc = scr->sc();

		if (scr->sc_frame() > images_slot->frame())
		{
			auto p = sc ? &sc->images() : nullptr;
			images_slot->set_data(&p);
		}
		bp->update();

		if (sc)
		{
			auto idx = app_frame() % fences.size();
			sc->acquire_image();
			fences[idx]->wait();
			d->gq->submit((Commandbuffer*)cbs[sc->image_index()], sc->image_avalible(), render_finished, fences[idx]);
			d->gq->present(sc, render_finished);
		}
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_foundation.typeinfo");
	typeinfo_load(L"flame_graphics.typeinfo");

	app.bp = BP::create_from_file(L"../renderpath/full_screen_shader/bp");
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = Window::create("", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.render_finished = Semaphore::create(app.d);

	auto& images = app.scr->sc()->images();

	app.fences.resize(images.size());
	app.cbs.resize(images.size());
	for (auto i = 0; i < images.size(); i++)
	{
		app.fences[i] = Fence::create(app.d);
		app.cbs[i] = Commandbuffer::create(app.d->gcp);
	}

	app.bp->set_graphics_device(app.d);
	
	app.images_slot = app.bp->find_input("rt.v");
	app.images_slot->set_data_p(&app.scr->sc()->images());
	app.bp->find_input("make_cmd.cmdbufs")->set_data_p(&app.cbs);

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
