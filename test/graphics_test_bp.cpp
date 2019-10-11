#include <flame/foundation/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<void*> cbs;
	BP* bp;
	Canvas* canvas;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_delay_events();

		bp->update();

		if (sc)
		{
			d->gq->submit({ (Commandbuffer*)cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}

		scr->signal = false;
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

	app.w = Window::create("Graphics Test", Vec2u(1280, 720), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app.bp->graphics_device = app.d;
	auto n_scr = app.bp->add_node(cH("graphics::SwapchainResizable"), "scr");
	n_scr->find_input("in")->set_data_p(app.scr);
	app.bp->find_input("*.rt_dst.type")->set_data_i(TargetImages);
	app.bp->find_input("*.rt_dst.v")->link_to(n_scr->find_output("images"));
	app.bp->find_input("*.make_cmd.cbs")->set_data_p(&app.cbs);
	{
		auto s_img_idx = app.bp->find_input("*.make_cmd.image_idx");
		if (s_img_idx)
			s_img_idx->link_to(n_scr->find_output("image_idx"));
	}
	app.bp->update();
	app.canvas = (Canvas*)app.bp->find_output("*.make_cmd.canvas")->data_p();

	looper().loop([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));
}
