#include <flame/serialize.h>
#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>

using namespace flame;
using namespace graphics;

struct App
{
	SysWindow* w;
	Device* d;
	Swapchain* sc;
	Renderpass* rp;
	Image* img;
	Descriptorlayout* dsl;
	Descriptorset* ds;
	Pipelinelayout* pll;
	Pipeline* pl;
	std::vector<Framebuffer*> fbs;
	std::vector<Commandbuffer*> cbs;
	Fence* fence;
	Semaphore* render_finished;

	void on_resize()
	{
		for (auto fb : fbs)
			Framebuffer::destroy(fb);
		for (auto cb : cbs)
			Commandbuffer::destroy(cb);
		auto image_count = sc->image_count();
		fbs.resize(image_count);
		for (auto i = 0; i < image_count; i++)
		{
			auto v = sc->image(i)->default_view();
			fbs[i] = Framebuffer::create(d, rp, 1, &v);
		}
		cbs.resize(image_count);
		auto vp = Vec4f(Vec2f(0.f), Vec2f(img->size));
		for (auto i = 0; i < image_count; i++)
		{
			auto cb = Commandbuffer::create(Commandpool::get_default(QueueGraphics));
			cb->begin();
			cb->begin_renderpass(fbs[i], 1, &Vec4f(0.f, 0.f, 0.f, 1.f));
			cb->set_viewport(vp);
			cb->set_scissor(vp);
			cb->bind_pipeline(pl);
			cb->bind_descriptorset(ds, 0, pll);
			cb->draw(3, 1, 0, 0);
			cb->end_renderpass();
			cb->end();
			cbs[i] = cb;
		}
	}

	void run()
	{
		if (!fbs.empty())
			sc->acquire_image();

		fence->wait();

		if (!cbs.empty())
		{
			Queue::get_default(QueueGraphics)->submit(1, &cbs[sc->image_index()], sc->image_avalible(), render_finished, fence);
			Queue::get_default(QueueGraphics)->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	{
		AttachmentInfo att;
		att.format = graphics::Swapchain::get_format();
		SubpassInfo sp;
		sp.color_attachment_count = 1;
		uint col_refs[] = {
			0
		};
		sp.color_attachments = col_refs;
		app.rp = Renderpass::create(app.d, 1, &att, 1, &sp, 0, nullptr);
	}
	app.img = Image::create_from_file(app.d, (engine_path / L"art/9.png").c_str());
	{
		DescriptorBinding db;
		db.type = DescriptorSampledImage;
		db.count = 1;
		db.name = "image";
		app.dsl = Descriptorlayout::create(app.d, 1, &db, true);
		app.ds = app.dsl->default_set();
		app.ds->set_image(0, 0, app.img->default_view(), Sampler::get_default(FilterLinear));
		app.pll = Pipelinelayout::create(app.d, 1, &app.dsl, 0);
		const wchar_t* shaders[] = {
			L"fullscreen.vert",
			L"blit.frag"
		};
		app.pl = Pipeline::create(app.d, (engine_path / L"shaders").c_str(), 2, shaders, app.pll, app.rp, 0);
	}
	app.on_resize();
	app.w->resize_listeners.add([](void*, const Vec2u& s) {
		app.on_resize();
		return true;
	}, Mail());
	app.fence = Fence::create(app.d);
	app.render_finished = Semaphore::create(app.d);

	looper().loop([](void*) {
		app.run();
	}, Mail());
}
