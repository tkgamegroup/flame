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

PerspectiveProjector projector(640.f, 360.f, 45.f, 1.f, 4.f);

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

	struct Drop
	{
		vec3 p;
		float sp;
		float end;

		Drop()
		{
			reset();
		}

		void reset()
		{
			p.x = (random() * 2.f - 1.f) * projector._screen_ratio;
			p.z = projector._near + (projector._far - projector._near) * random();
			end = -p.z * projector._tan_fovy;
			p.y = -end + 0.1;
			sp = random();
		}

		void fall()
		{
			sp += 0.8f * dt;
			p.y -= sp * dt;
			if (p.y < end)
				reset();
		}

		void show(Canvas* canvas)
		{
			canvas->begin_path();
			auto c1 = projector.project(p);
			auto c2 = projector.project(p - vec3(0.f, 0.1f, 0.f));
			canvas->move_to(c1.x, c1.y);
			canvas->line_to(c2.x, c2.y);
			canvas->stroke(cvec4(83, 209, 227, 255), 4.f / p.z);
		}
	};

	std::vector<Drop> drops;

	void setup()
	{
		srand(time(0));

		drops.resize(3000);
		for (auto& d : drops)
			d.p.y = random() * d.end * 2.f - d.end;
	}

	void run(float delta_time)
	{
		dt = delta_time;

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
	app.canvas->set_clear_color(cvec4(230, 230, 250, 1.f));
	app.on_resize();

	app.setup();

	looper().loop([](Capture&, float delta_time) {
		app.run(delta_time);
	}, Capture());

	return 0;
}
