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

	FontAtlas* font_atlas;

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
	app.w = Window::create("Tetris", Vec2u(800, 600), WindowFrame);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->image_count());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);
	TypeinfoDatabase::load(L"flame_foundation.typeinfo", true, true);
	TypeinfoDatabase::load(L"flame_graphics.typeinfo", true, true);
	TypeinfoDatabase::load(L"flame_universe.typeinfo", true, true);

	app.u = Universe::create();
	app.u->add_object(app.w);

	auto w = World::create(app.u);
	w->add_system(sLayoutManagement::create());
	app.s_event_dispatcher = sEventDispatcher::create();
	w->add_system(app.s_event_dispatcher);
	auto s_2d_renderer = s2DRenderer::create(L"../renderpath/canvas/bp", app.scr, FLAME_CHASH("SwapchainResizable"), &app.cbs);
	w->add_system(s_2d_renderer);

	auto atlas = Atlas::load(app.d, L"../game/tetris/release/main.png");
	{
		auto canvas = s_2d_renderer->canvas;
		wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc"
		};
		app.font_atlas = FontAtlas::create(app.d, FontDrawPixel, 1, fonts);
		canvas->add_font(app.font_atlas);

		canvas->add_atlas(atlas);
	}

	auto root = w->root();
	ui::set_current_entity(root);
	app.c_element_root = ui::c_element();
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas);
	ui::set_current_root(root);

	ui::push_parent(root);

		ui::e_empty();
		ui::next_element_pos = Vec2f(20.f);
		ui::next_element_size = Vec2f(16.f * 10, 16.f * 24);
		ui::c_element();
		{
			auto c_tile_map = cTileMap::create();
			c_tile_map->cell_size = Vec2f(16.f);
			c_tile_map->set_size(Vec2u(10, 24));
			for (auto i = 0; i < atlas->tile_count(); i++)
				c_tile_map->add_tile((atlas->canvas_slot_ << 16) + i);
			c_tile_map->set_cell(Vec2u(0), 5);
			ui::current_entity()->add_component(c_tile_map);
		}

		ui::e_text(L"");
		ui::c_aligner(AlignxRight, AlignyBottom);
		add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
		}, new_mail_p(ui::current_entity()->get_component(cText)));

	ui::pop_parent();

	looper().loop([](void* c) {
		(*(App**)c)->run();
	}, new_mail_p(&app));

	return 0;
}
