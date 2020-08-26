 #include <flame/graphics/device.h>
#include <flame/graphics/image.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/command.h>
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
	std::vector<CommandBuffer*> cbs;
	Semaphore* render_finished;

	Canvas* canvas;

	FontAtlas* font_atlas;

	void on_resize()
	{
		std::vector<ImageView*> vs(sc->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = sc->get_image(i)->get_default_view();

		canvas->set_target(vs.size(), vs.data());

		for (auto cb : cbs)
			cb->release();
		cbs.resize(vs.size());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i] = CommandBuffer::create(d->get_command_pool(QueueGraphics));
	}

	void run()
	{
		if (!cbs.empty())
		{
			sc->acquire_image();

			auto img_idx = sc->get_image_index();
			auto cb = cbs[img_idx];

			canvas->prepare();

			canvas->begin_path();
			canvas->move_to(150.f, 20.f);
			canvas->line_to(120.f, 70.f);
			canvas->line_to(190.f, 80.f);
			canvas->fill(Vec4c(255), true);

			auto y = 50.f;
			for (auto i = 0; i < 20; i++)
			{
				canvas->begin_path();
				canvas->move_to(20.f, y);
				canvas->line_to(50.f, y);
				canvas->line_to(60.f, y - 20.f);
				canvas->line_to(70.f, y + 20.f);
				canvas->stroke(Vec4c(255), (i + 1) * 0.5f, true);
				y += 45.f;
			}

			//canvas->add_text(0, L"Hello World  ", 14, Vec2f(5, 0), Vec4c(162, 21, 21, 255));

			canvas->record(cb, img_idx);

			fence->wait();

			if (!cbs.empty())
			{
				auto q = d->get_queue(QueueGraphics);
				q->submit(1, &cb, sc->get_image_avalible(), render_finished, fence);
				q->present(sc, render_finished);
			}
		}
	}

}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");
	set_engine_path(engine_path.c_str());

	app.w = Window::create("Graphics Test", Vec2u(1280, 960), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.sc = Swapchain::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.canvas = Canvas::create(app.d);
	app.canvas->set_clear_color(Vec4c(100, 100, 100, 255));
	{
		Font* fonts[] = {
			Font::create(L"c:/windows/fonts/consola.ttf"),
			Font::create((engine_path / L"assets/font_awesome.ttf").c_str())
		};
		app.font_atlas = FontAtlas::create(app.d, 2, fonts);
	}
	app.canvas->set_resource(-1, app.font_atlas->get_view(), app.d->get_sampler(FilterNearest), "", nullptr, app.font_atlas);
	app.on_resize();
	app.w->add_resize_listener([](Capture&, const Vec2u&) {
		app.on_resize();
	}, Capture());

	looper().loop([](Capture&, float) {
		app.run();
	}, Capture());

	return 0;
}
