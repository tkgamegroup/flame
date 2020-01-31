#include <flame/serialize.h>
#include <flame/foundation/bitmap.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

struct App
{
	Window* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	Semaphore* render_finished;
	std::vector<TypeinfoDatabase*> dbs;

	Universe* u;
	sEventDispatcher* s_event_dispatcher;
	cElement* c_element_root;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		c_element_root->set_size(Vec2f(w->size));
		u->update();

		if (sc)
		{
			d->gq->submit(1, &cbs[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char **args)
{
	app.w = Window::create("Tetris", Vec2u(800, 400), WindowFrame);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->image_count());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_foundation.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_graphics.typeinfo"));
	app.dbs.push_back(TypeinfoDatabase::load(app.dbs, L"flame_universe.typeinfo"));

	app.u = Universe::create();
	app.u->add_object(app.w);

	auto w = World::create(app.u);
	w->add_system(sLayoutManagement::create());
	app.s_event_dispatcher = sEventDispatcher::create();
	w->add_system(app.s_event_dispatcher);
	auto s_2d_renderer = s2DRenderer::create(L"../renderpath/canvas/bp", app.scr, FLAME_CHASH("SwapchainResizable"), &app.cbs);
	w->add_system(s_2d_renderer);
	load_res(w, app.dbs, L"../game/tetris/res");
	auto font_atlas_joystix = (FontAtlas*)w->find_object(FLAME_CHASH("FontAtlas"), 0);
	auto font_atlas_standard = FontAtlas::create(app.d, FontDrawPixel, 1, &L"c:/windows/fonts/msyh.ttc");
	s_2d_renderer->canvas->add_font(font_atlas_standard);

	auto atlas_main = (Atlas*)w->find_object(FLAME_CHASH("Atlas"), FLAME_CHASH("release/main.png"));

	auto brick_idx = atlas_main->find_region(FLAME_CHASH("../asset/brick.png"));
	auto block_idx = atlas_main->find_region(FLAME_CHASH("../asset/block.png"));

	auto main_scene = Entity::create_from_file(w, app.dbs, L"../game/tetris/main.prefab");

	auto root = w->root();
	{
		app.c_element_root = cElement::create();
		root->add_component(app.c_element_root);

		root->add_component(cLayout::create(LayoutFree));
	}

	root->add_child(main_scene);

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		auto c_text = cText::create(font_atlas_standard);
		e_fps->add_component(c_text);

		auto c_aligner = cAligner::create();
		c_aligner->x_align_ = AlignxRight;
		c_aligner->y_align_ = AlignyBottom;
		e_fps->add_component(c_aligner);

		add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
		}, new_mail_p(c_text));
	}

	looper().loop([](void* c) {
		(*(App**)c)->run();
	}, new_mail_p(&app));

	return 0;
}
