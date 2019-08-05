#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

using namespace flame;

struct App
{
	Window* w;
	graphics::Device* d;
	graphics::Semaphore* render_finished;
	BP* bp;
	AttributeP<void>* psc;
	std::vector<graphics::Fence*> fences;
	AttributeV<std::vector<void*>>* cbs;

	void run()
	{
		auto sc = (graphics::Swapchain*)psc->v;
		if (sc)
		{
			auto idx = app_frame() % fences.size();
			if (!cbs->v.empty())
			{
				sc->acquire_image();
				fences[idx]->wait();
				d->gq->submit((graphics::Commandbuffer*)((cbs->v)[sc->image_index()]), sc->image_avalible(), render_finished, fences[idx]);
				d->gq->present(sc, render_finished);
			}
		}

		bp->update();
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"flame_foundation.typeinfo");
	typeinfo_load(L"flame_graphics.typeinfo");

	app.bp = BP::create_from_file(L"../renderpath/clear_screen/bp");
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = Window::create("", Vec2u(1280, 720), WindowFrame);
	app.d = graphics::Device::create(true);
	app.render_finished = graphics::Semaphore::create(app.d);

	app.bp->set_graphics_device(app.d);
	app.bp->find_input("scr.window")->set_data(&app.w);
	app.bp->update();

	app.psc = (AttributeP<void>*)app.bp->find_output("scr.sc")->data();
	app.cbs = (AttributeV<std::vector<void*>>*)app.bp->find_output("cbs.out")->data();

	auto sc = (graphics::Swapchain*)app.psc->v;
	assert(sc);
	app.fences.resize(sc->images().size());
	for (auto i = 0; i < app.fences.size(); i++)
		app.fences[i] = graphics::Fence::create(app.d);

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
