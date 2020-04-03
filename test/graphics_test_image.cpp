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
	RenderpassAndFramebuffer* rnf;
	Image* img;
	Descriptorlayout* dsl;
	Pipelinelayout* pll;
	Pipeline* pl;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;

	void run()
	{
		if (sc->image_count())
			sc->acquire_image();

		fence->wait();

		if (sc->image_count())
		{
			d->gq->submit(1, &cbs.v[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
	TypeinfoDatabase::load(L"flame_graphics.dll", true, true);

	std::filesystem::path engine_path = getenv("FLAME_PATH");


	app.w = SysWindow::create("Graphics Test", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.sc = Swapchain::create(app.d, app.w);
	{
		Array<Image*> images;
		images.resize(app.sc->image_count());
		for (auto i = 0; i < images.s; i++)
			images[i] = app.sc->image(i);
		RenderTarget rt;
		rt.type = TargetImages;
		rt.v = &images;
		rt.clear = true;
		rt.clear_color = Vec4c(138, 213, 241, 255);
		auto p_rt = &rt;
		SubpassTargetInfo sp;
		sp.color_target_count = 1;
		sp.color_targets = &p_rt;
		sp.depth_target = nullptr;
		sp.resolve_target_count = 0;
		sp.resolve_targets = nullptr;
		auto p_sp = &sp;
		app.rnf = RenderpassAndFramebuffer::create(app.d, 1, &p_sp);
		app.img = Image::create_from_file(app.d, (engine_path / L"art/9.png").c_str());
		DescriptorBinding db;
		db.type = DescriptorSampledImage;
		db.count = 1;
		db.name = "image";
		db.buffer = nullptr;
		db.view = Imageview::create(app.img);
		db.sampler = app.d->sp_linear;
		auto p_db = &db;
		app.dsl = Descriptorlayout::create(app.d, 1, &p_db, true);
		app.pll = Pipelinelayout::create(app.d, 1, &app.dsl, 0);
		const wchar_t* shaders[] = {
			L"fullscreen.vert",
			L"image.frag"
		};
		app.pl = Pipeline::create(app.d, (engine_path / L"shaders").c_str(), 2, shaders, app.pll, app.rnf->renderpass(), 0);
	}
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.sc->image_count());
	auto size = app.img->size;
	auto vp = Vec4f(Vec2f(0.f), Vec2f(size));
	for (auto i = 0; i < app.cbs.s; i++)
	{
		auto cb = Commandbuffer::create(app.d->gcp);
		app.cbs.v[i] = cb;
		cb->begin();
		cb->begin_renderpass(app.rnf->framebuffer(i), app.rnf->clearvalues());
		cb->set_viewport(vp);
		cb->set_scissor(vp);
		cb->bind_pipeline(app.pl);
		cb->bind_descriptorset(app.dsl->default_set(), 0, app.pll);
		cb->draw(3, 1, 0, 0);
		cb->end_renderpass();
		cb->end();
	}
	app.render_finished = Semaphore::create(app.d);

	looper().loop([](void*) {
		app.run();
	}, Mail());
}
