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
	std::vector<Fence*> fences;
	std::vector<void*> cbs;
	BP* bp;

	void run()
	{
		bp->update();

		auto sc = scr->sc();
		if (sc)
		{
			auto idx = looper().frame % fences.size();
			sc->acquire_image();
			fences[idx]->wait();
			looper().process_delay_events();

			d->gq->submit({ (Commandbuffer*)cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fences[idx]);
			d->gq->present(sc, render_finished);
		}
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	app.bp = BP::create_from_file(L"../renderpath/clear_screen/bp", true);
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

	app.bp->graphics_device = app.d;
	auto scr_n = app.bp->add_node(cH("graphics::SwapchainResizable"), "scr");
	scr_n->find_input("in")->set_data_p(app.scr);
	app.bp->find_input("rt_dst.type")->set_data_i(TargetImages);
	app.bp->find_input("rt_dst.v")->link_to(scr_n->find_output("images"));
	app.bp->find_input("make_cmd.cbs")->set_data_p(&app.cbs);

	looper().loop([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail_p(&app));
}
