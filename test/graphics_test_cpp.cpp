#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>

namespace flame
{
	struct CanvasShaderPushconstantT$
	{
		Vec2f scale$;
		Vec2f sdf_range$;
	}unused;
}

using namespace flame;

struct App
{
	Window* w;
	graphics::Device* d;
	graphics::Semaphore* render_finished;
	graphics::Swapchain* sc;
	std::vector<graphics::Fence*> fences;
	std::vector<graphics::Commandbuffer*> cbs;

	graphics::Canvas* canvas;
	uint font_atlas_index;

	void run()
	{
		auto idx = app_frame() % fences.size();
		sc->acquire_image();
		fences[idx]->wait();

		canvas->add_text(font_atlas_index, Vec2f(100, 50), Vec4c(200, 160, 230, 255), L"Hello World");

		auto cb = cbs[sc->image_index()];
		canvas->record(cb);
		
		d->gq->submit(cb, sc->image_avalible(), render_finished, fences[idx]);
		d->gq->present(sc, render_finished);
	}

}app;
auto papp = &app;

int main(int argc, char** args)
{
	typeinfo_load(L"graphics_test_cpp.typeinfo");

	app.w = Window::create("", Vec2u(1280, 720), WindowFrame);
	app.d = graphics::Device::create(true);
	app.render_finished = graphics::Semaphore::create(app.d);
	app.sc = graphics::Swapchain::create(app.d, app.w);
	app.fences.resize(app.sc->images().size());
	for (auto& f : app.fences)
		f = graphics::Fence::create(app.d);

	app.canvas = graphics::Canvas::create(app.d, app.sc);

	auto font_msyh = graphics::Font::create(L"c:/windows/fonts/msyh.ttc", 16);
	auto font_atlas = graphics::FontAtlas::create(app.d, 16, false, { font_msyh });
	app.font_atlas_index = app.canvas->add_font_atlas(font_atlas);

	//graphics::SubpassTargetInfo subpass;
	//graphics::SubpassTarget target(&app.sc->images(), true, Vec4c(130, 100, 200, 255));
	//subpass.color_targets.emplace_back(&target);
	//auto rnf = graphics::RenderpassAndFramebuffer::create(app.d, { &subpass });
	//auto pll = graphics::Pipelinelayout::create(app.d, {}, 0, cH("CanvasShaderPushconstantT"));
	//auto vert = graphics::Shader::create(app.d, L"../renderpath/canvas/element.vert", "", pll);
	app.cbs.resize(app.sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
	{
		auto cb = graphics::Commandbuffer::create(app.d->gcp);
		//cb->begin();
		//cb->begin_renderpass(rnf->renderpass(), (graphics::Framebuffer*)rnf->framebuffers()[i], rnf->clearvalues());
		//cb->end_renderpass();
		//cb->end();
		app.cbs[i] = cb;
	}

	auto thiz = &app;
	app_run([](void* c) {
		auto app = (*(App * *)c);
		app->run();
	}, new_mail(&thiz));
}
