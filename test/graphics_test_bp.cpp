#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;
	BP* bp;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		bp->update();

		if (sc)
		{
			d->gq->submit({ cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	app.bp = BP::create_from_file(L"../renderpath/logo/bp", false);
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = Window::create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);

	app.scr->link_bp(app.bp, app.cbs);
	app.bp->update();

	looper().loop([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));
}
