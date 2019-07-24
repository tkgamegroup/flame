#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpath.h>

using namespace flame;

struct App
{
	Window* w;
	graphics::Device* d;
	graphics::Semaphore* render_finished;
	graphics::Swapchain* sc;
	std::vector<graphics::Commandbuffer*> cbs;

	void run()
	{
		auto idx = app_frame() % sc->image_count();
		auto fence = sc->fence(idx);
		sc->acquire_image();
		fence->wait();
		d->gq->submit(cbs[sc->image_index()], sc->image_avalible(), render_finished, fence);
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

	std::vector<graphics::Image*> swapchain_images;
	swapchain_images.resize(app.sc->image_count());
	for (auto i = 0; i < swapchain_images.size(); i++)
		swapchain_images[i] = app.sc->image(i);

	graphics::RenderpathInfo rp_info;
	{
		graphics::RenderpathPassTarget target(&swapchain_images, true, Vec4c(130, 100, 200, 255));
		graphics::RenderpathPassInfo pass_info;
		pass_info.color_targets.push_back(&target);
		rp_info.passes.push_back(pass_info);
	}
	auto rp = graphics::Renderpath::create(app.d, rp_info);
	app.cbs.resize(rp->image_count());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = rp->commandbuffer(i);

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
