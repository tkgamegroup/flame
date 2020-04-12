#include <flame/foundation/foundation.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/image.h>

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Renderpass* rp;
	std::vector<Framebuffer*> fbs;
	std::vector<Commandbuffer*> cbs;
	Fence* fence;
	Semaphore* render_finished;

	void on_resize()
	{
		for (auto fb : fbs)
			Framebuffer::destroy(fb);
		for (auto cb : cbs)
			Commandbuffer::destroy(cb);
		auto image_count = sc->image_count();
		fbs.resize(image_count);
		for (auto i = 0; i < image_count; i++)
		{
			auto v = sc->image(i)->default_view();
			fbs[i] = Framebuffer::create(d, rp, 1, &v);
		}
		cbs.resize(image_count);
		for (auto i = 0; i < image_count; i++)
		{
			auto cb = Commandbuffer::create(d->gcp);
			cb->begin();
			cb->begin_renderpass(app.fbs[i], 1, &Vec4f(0.23f, 0.44f, 0.75f, 1.f));
			cb->end_renderpass();
			cb->end();
			cbs[i] = cb;
		}
	}

	void run()
	{
		if (!fbs.empty())
			sc->acquire_image();

		fence->wait();

		if (!cbs.empty())
		{
			d->gq->submit(1, &cbs[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	{
		AttachmentInfo att;
		att.format = graphics::Swapchain::get_format();
		SubpassInfo sp;
		sp.color_attachment_count = 1;
		uint col_refs[] = {
			0
		};
		sp.color_attachments = col_refs;
		app.rp = Renderpass::create(app.d, 1, &att, 1, &sp, 0, nullptr);
	}
	app.fence = Fence::create(app.d);
	app.render_finished = Semaphore::create(app.d);
	app.on_resize();
	app.w->resize_listeners.add([](void*, const Vec2u&) {
		app.on_resize();
		return true;
	}, Mail());

	looper().loop([](void*) {
		app.run();
	}, Mail());
}
