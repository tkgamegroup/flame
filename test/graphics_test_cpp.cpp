#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

using namespace flame;

struct App
{
	Window* w;
	graphics::Device* d;
	graphics::Semaphore* render_finished;
	graphics::Swapchain* sc;
	std::vector<graphics::Fence*> fences;
	std::vector<graphics::Commandbuffer*> cbs;

	void run()
	{
		auto idx = app_frame() % fences.size();
		sc->acquire_image();
		fences[idx]->wait();
		d->gq->submit(cbs[sc->image_index()], sc->image_avalible(), render_finished, fences[idx]);
		d->gq->present(sc, render_finished);
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	app.w = Window::create("", Vec2u(1280, 720), WindowFrame);
	app.d = graphics::Device::create(true);
	app.render_finished = graphics::Semaphore::create(app.d);
	app.sc = graphics::Swapchain::create(app.d, app.w);
	app.fences.resize(app.sc->images().size());
	for (auto& f : app.fences)
		f = graphics::Fence::create(app.d);

	graphics::SubpassTargetInfo subpass;
	graphics::SubpassTarget target(&app.sc->images(), true, Vec4c(130, 100, 200, 255));
	subpass.color_targets.emplace_back(&target);
	auto rnf = graphics::RenderpassAndFramebuffer::create(app.d, { &subpass });
	app.cbs.resize(app.sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
	{
		auto cb = graphics::Commandbuffer::create(app.d->gcp);
		cb->begin();
		cb->begin_renderpass(rnf->renderpass(), (graphics::Framebuffer*)rnf->framebuffers()[i], rnf->clearvalues());
		cb->end_renderpass();
		cb->end();
		app.cbs[i] = cb;
	}

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
