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
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<void*> cbs;
	BP* bp;

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
	app.bp = BP::create_from_file(L"../renderpath/canvas_make_cmd/bp", true);
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

	app.fence = Fence::create(app.d);
	app.cbs.resize(images.size());
	for (auto i = 0; i < images.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app.bp->graphics_device = app.d;
	auto n_scr = app.bp->add_node(cH("graphics::SwapchainResizable"), "scr");
	n_scr->find_input("in")->set_data_p(app.scr);
	app.bp->find_input("*.rt_dst.type")->set_data_i(TargetImages);
	app.bp->find_input("*.rt_dst.v")->link_to(n_scr->find_output("images"));
	app.bp->find_input("*.make_cmd.cbs")->set_data_p(&app.cbs);
	{
		auto s_img_idx = app.bp->find_input("make_cmd.image_idx");
		if (s_img_idx)
			s_img_idx->link_to(n_scr->find_output("image_idx"));
	}

	looper().loop([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));
}
