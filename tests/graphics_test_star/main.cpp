﻿#include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/command.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

using namespace flame;
using namespace graphics;

PerspectiveProjector projector(500.f, 500.f, 45.f, 1.f, 4.f);

float dt;

struct App
{
	Window* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	std::vector<CommandBuffer*> cbs;
	Semaphore* render_finished;

	Canvas* canvas;

	void on_resize()
	{
		std::vector<ImageView*> vs(sc->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = sc->get_image(i)->get_default_view();

		canvas->set_target(vs.size(), vs.data());

		cbs.resize(vs.size());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i] = CommandBuffer::create(d->get_command_pool(QueueGraphics));
	}

	struct Star
	{
		vec3 p;

		Star()
		{
			reset();
		}

		void reset()
		{
			p.x = (random() * 2.f - 1.f) * 4.f;
			p.y = (random() * 2.f - 1.f) * 4.f;
			p.z = projector._far;
		}

		void move()
		{
			p.z -= 1.5f * dt;
			if (p.z <= projector._near)
				reset();
		}

		void show(Canvas* canvas)
		{
			canvas->begin_path();
			auto r = 4.f / p.z;
			auto c = projector.project(p);
			canvas->move_to(c.x + r, c.y);
			for (auto i = 60; i < 360; i += 60)
			{
				auto rad = radians(i);
				canvas->line_to(c.x + cos(rad) * r, c.y + sin(rad) * r);
			}
			canvas->fill(cvec4(255, 255, 255, 80 * (3.f - p.z + 1.f)));
		}
	};

	std::vector<Star> stars;

	void setup()
	{
		srand(time(0));

		stars.resize(1000);
		for (auto& s : stars)
			s.p.z = random() * (projector._far - projector._near) + projector._near;
	}

	void run(float delta_time)
	{
		dt = delta_time;

		if (!cbs.empty())
			sc->acquire_image();

		auto img_idx = sc->get_image_index();
		auto cb = cbs[img_idx];

		canvas->prepare();

		for (auto& s : stars)
		{
			s.move();
			s.show(canvas);
		}

		canvas->record(cb, img_idx);

		fence->wait();

		if (!cbs.empty())
		{
			auto q = d->get_queue(QueueGraphics);
			q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
			q->present(sc, render_finished);
		}
	}

}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	set_engine_path(engine_path.c_str());

	app.w = Window::create("Graphics Test", uvec2(projector._screen_width, projector._screen_height), WindowFrame);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.canvas = Canvas::create(app.d);
	app.canvas->set_clear_color(cvec4(0, 0, 0, 1.f));
	app.on_resize();

	app.setup();

	looper().loop([](Capture&, float delta_time) {
		app.run(delta_time);
	}, Capture());

	return 0;
}
