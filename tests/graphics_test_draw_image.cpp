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
	std::vector<Framebuffer*> fbs;
	std::vector<Commandbuffer*> cbs;
	Image* img;
	Descriptorlayout* dsl;
	Descriptorset* ds;
	Pipelinelayout* pll;
	Pipeline* pl;
	Fence* fence;
	Semaphore* render_finished;

	void create_framebuffers()
	{
		for (auto fb : fbs)
			Framebuffer::destroy(fb);
		for (auto cb : cbs)
			Commandbuffer::destroy(cb);
		auto image_count = sc->image_count();
		if (image_count > 0)
		{
			fbs.resize(image_count);
			for (auto i = 0; i < image_count; i++)
			{
				auto v = sc->image(i)->default_view();
				fbs[i] = Framebuffer::create(d, rp, 1, &v);
			}
			cbs.resize(image_count);
			for (auto i = 0; i < image_count; i++)
				cbs[i] = Commandbuffer::create(d->gcp);
		}
	}

	void run()
	{
		if (!fbs.empty())
			sc->acquire_image();

		fence->wait();

		if (!cbs.empty())
		{
			d->gq->submit(1, &cbs[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame);
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
	app.create_framebuffers();
	app.img = Image::create_from_file(app.d, (engine_path / L"art/9.png").c_str());
	{
		DescriptorBinding db;
		db.type = DescriptorSampledImage;
		db.count = 1;
		db.name = "image";
		app.dsl = Descriptorlayout::create(app.d, 1, &db, true);
		app.ds = app.dsl->default_set();
		app.ds->set_image(0, 0, app.img->default_view(), app.d->sp_linear);
		app.pll = Pipelinelayout::create(app.d, 1, &app.dsl, 0);
		const wchar_t* shaders[] = {
			L"fullscreen.vert",
			L"blit.frag"
		};
		app.pl = Pipeline::create(app.d, (engine_path / L"shaders").c_str(), 2, shaders, app.pll, app.rp, 0);
	}
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.sc->image_count());
	auto size = app.img->size;
	auto vp = Vec4f(Vec2f(0.f), Vec2f(size));
	for (auto i = 0; i < app.cbs.size(); i++)
	{
		auto cb = app.cbs[i];
		cb->begin();
		cb->begin_renderpass(app.fbs[i], 1, &Vec4f(0.f, 0.f, 0.f, 1.f));
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		cb->bind_pipeline(app.pl);
		cb->bind_descriptorset(app.ds, 0, app.pll);
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();
		cb->end();
	}
	app.render_finished = Semaphore::create(app.d);

	looper().loop([](void*) {
		app.run();
	}, Mail());
}
