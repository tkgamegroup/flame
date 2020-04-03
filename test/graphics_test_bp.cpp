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
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;
	BP* bp;

	void run()
	{
		if (sc->image_count())
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		bp->update();

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

	app.bp = BP::create_from_file((std::filesystem::path(getenv("FLAME_PATH")) / L"renderpath/clear/bp").c_str());
	if (!app.bp)
	{
		printf("bp not found, exit\n");
		return 0;
	}

	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.sc->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
		app.cbs.v[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);

	app.sc->link_bp(app.bp, &app.cbs);

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
