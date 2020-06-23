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

const auto Width = 640;
const auto Height = 360;
const auto Ratio = (float)Width / (float)Height;
const auto Fovy = 45.f;
const auto TanFovy = tan(Fovy * ANG_RAD);
const auto Focus = 1.f;
const auto Far = 4.f;

float random()
{
	return (float)rand() / (float)RAND_MAX;
}

float dt;

struct App
{
	Window* w;
	Device* d;
	Swapchain* sc;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;

	graphics::Canvas* canvas;

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

	struct Drop
	{
		Vec3f p;
		float sp;
		float end;

		Drop()
		{
			reset();
		}

		void reset()
		{
			p.x() = (random() * 2.f - 1.f) * Ratio;
			p.z() = Focus + (Far - Focus) * random();
			end = -p.z() * TanFovy;
			p.y() = -end + 0.1;
			sp = random();
		}

		void fall()
		{
			sp += 0.8f * dt;
			p.y() -= sp * dt;
			if (p.y() < end)
				reset();
		}

		void project(Vec3f& p)
		{
			p.x() = (p.x() / p.z() * Ratio + 1.f) * 0.5f * Width;
			p.y() = (p.y() / p.z() * -1.f + 1.f) * 0.5f * Height;
		}

		void show(graphics::Canvas* canvas)
		{
			auto p1 = p;
			auto p2 = p;
			p2.y() -= 0.1;
			
			project(p1);
			project(p2);
			Vec2f points[] = {
				Vec2f(p1),
				Vec2f(p2),
			};

			canvas->stroke(2, points, Vec4c(138, 43, 226, 255), 4.f / p.z());
		}
	};

	std::vector<Drop> drops;

	void setup()
	{
		srand(time(0));

		drops.resize(3000);
	}

	void run()
	{
		dt = get_looper()->get_delta_time();

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

	app.w = Window::create("Graphics Test", Vec2u(Width, Height), WindowFrame);
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
