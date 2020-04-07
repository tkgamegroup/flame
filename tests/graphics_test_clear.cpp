#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	RenderpassAndFramebuffer* rnf;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;

	void run()
	{
		if (sc->image_count())
			sc->acquire_image();

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
	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	{
		Array<Image*> images;
		images.resize(app.sc->image_count());
		for (auto i = 0; i < images.s; i++)
			images[i] = app.sc->image(i);
		RenderTarget rt;
		rt.type = TargetImages;
		rt.v = &images;
		rt.clear = true;
		rt.clear_color = Vec4c(138, 213, 241, 255);
		auto p_rt = &rt;
		SubpassTargetInfo sp;
		sp.color_target_count = 1;
		sp.color_targets = &p_rt;
		sp.depth_target = nullptr;
		sp.resolve_target_count = 0;
		sp.resolve_targets = nullptr;
		auto p_sp = &sp;
		app.rnf = RenderpassAndFramebuffer::create(app.d, 1, &p_sp);
	}
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.sc->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
	{
		auto cb = Commandbuffer::create(app.d->gcp);
		app.cbs.v[i] = cb;
		cb->begin();
		cb->begin_renderpass(app.rnf->framebuffer(i), app.rnf->clearvalues());
		cb->end_renderpass();
		cb->end();
	}
	app.render_finished = Semaphore::create(app.d);

	looper().loop([](void*) {
		app.run();
	}, Mail());
}
