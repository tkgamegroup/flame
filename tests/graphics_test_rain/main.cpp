#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;

	graphics::Canvas* canvas;

	struct Drop
	{
		Vec2f p = Vec2f(0.f);
		float yspeed = 1;

		Drop()
		{
			p.x() = rand() % 640;
			p.y() = rand() % 100 - 200;
			yspeed = (rand() % 60) / 10.f + 4.f;
		}

		void fall()
		{
			p.y() += yspeed;
			if (p.y() > 360.f)
				p.y() = rand() % 100 - 200;
		}

		void show(graphics::Canvas* canvas)
		{
			std::vector<Vec2f> points;
			path_rect(points, Vec2f(p.x() - 1.f, p.y()), Vec2f(2.f, 10.f));
			canvas->fill(points.size(), points.data(), Vec4c(138, 43, 226, 255));
		}
	};

	std::vector<Drop> drops;

	void on_resize()
	{
		std::vector<Imageview*> vs(sc->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = sc->get_image(i)->get_default_view();

		canvas->set_target(vs.size(), vs.data());

		cbs.resize(vs.size());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i] = Commandbuffer::create(d->get_graphics_commandpool());
	}

	void setup()
	{
		for (auto i = 0; i < 500; i++)
		{
			Drop d;
			drops.push_back(d);
		}
	}

	void run()
	{
		if (!cbs.empty())
			sc->acquire_image();

		auto img_idx = sc->get_image_index();
		auto cb = cbs[img_idx];

		canvas->prepare();

		for (auto& d : drops)
		{
			d.fall();
			d.show(canvas);
		}

		canvas->record(cb, img_idx);

		fence->wait();

		if (!cbs.empty())
		{
			auto q = d->get_graphics_queue();
			q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
			q->present(sc, render_finished);
		}
	}

}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	set_engine_path(engine_path.c_str());

	app.w = Window::create("Graphics Test", Vec2u(640, 360), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.canvas = Canvas::create(app.d);
	app.canvas->set_clear_color(Vec4c(230, 230, 250, 1.f));
	app.on_resize();

	app.setup();

	get_looper()->loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
