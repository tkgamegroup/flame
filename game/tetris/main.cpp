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

#include "mino.h"

const auto board_width = 10U;
const auto board_height = 24U;
const auto down_ticks = 24U;

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
	Entity* root;
	cElement* c_element_root;

	cTileMap* board;

	float time;
	bool gaming;
	int mino_y;
	int mino_x;
	uint mino_id;
	uint mino_rotation;
	uint mino_ticks;

	App()
	{
		init_minos();

		time = 0.f;
		gaming = true;
		mino_y = -1;
		mino_x = 0;
		mino_rotation = 0;
		mino_ticks = 0;
	}

	void toggle_board(int id, uint rotaion, int px, int py, bool on)
	{
		auto tile = on ? id : -1;
		for (auto x = 0; x < 4; x++)
		{
			for (auto y = 0; y < 4; y++)
			{
				if (mino_datas[id].tiles[rotaion][x][y])
					board->set_cell(Vec2u(x + px, y + py), tile);
			}
		}
	}

	bool check_board(int id, uint rotaion, int px, int py)
	{
		for (auto x = 0; x < 4; x++)
		{
			for (auto y = 0; y < 4; y++)
			{
				if (mino_datas[id].tiles[rotaion][x][y])
				{
					auto xx = x + px;
					auto yy = y + py;
					if (xx < 0 || xx >= board_width || yy < 0 || yy >= board_height)
						return false;
					if (board->cell(Vec2u(xx, yy)) != -1)
						return false;
				}
			}
		}
		return true;
	}

	bool line_empty(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2u(x, l)) != -1)
				return false;
		}
		return true;
	}

	bool line_full(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2u(x, l)) == -1)
				return false;
		}
		return true;
	}

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		time += looper().delta_time;
		const auto frame_rate = 1.f / 2400.f;
		while (time > frame_rate)
		{
			if (gaming)
			{
				if (mino_y != -1)
					toggle_board(mino_id, mino_rotation, mino_x, mino_y, false);

				if (mino_y == -1)
				{
					mino_x = MinoData::start_x;
					mino_y = MinoData::start_y;
					mino_id = rand() % array_size(mino_datas);
					mino_rotation = 0;
					mino_ticks = 0;
				}

				if (mino_ticks == down_ticks)
				{
					if (check_board(mino_id, mino_rotation, mino_x, mino_y + 1))
						mino_y++;
					else
					{
						toggle_board(mino_id, mino_rotation, mino_x, mino_y, true);
						if (!line_empty(3))
						{
							gaming = false;
							ui::e_begin_dialog(Vec4c(100));
							ui::e_text(L"Game Over");
							ui::e_button(Icon_REPEAT, [](void* c) {
								auto thiz = *(App**)c;
								ui::remove_top_layer(thiz->root);
								thiz->board->clear_cells();
								thiz->gaming = true;
							}, new_mail_p(this));
							ui::e_end_dialog();
						}
						mino_y = -1;
					}
					mino_ticks = 0;
				}
				mino_ticks++;

				if (mino_y != -1)
					toggle_board(mino_id, mino_rotation, mino_x, mino_y, true);
			}

			time -= frame_rate;
		}

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
			L"c:/windows/fonts/msyh.ttc",
			L"../art/font_awesome.ttf"
		};
		app.font_atlas = FontAtlas::create(app.d, FontDrawPixel, 2, fonts);
		canvas->add_font(app.font_atlas);

		canvas->add_atlas(atlas);
	}

	auto root = w->root();
	app.root = root;
	ui::set_current_entity(root);
	app.c_element_root = ui::c_element();
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas);
	ui::set_current_root(root);

	ui::push_parent(root);

		ui::e_empty();
		ui::next_element_pos = Vec2f(20.f);
		ui::next_element_size = Vec2f(16.f * board_width, 16.f * (board_height - 3.8f));
		{
			auto ce = ui::c_element();
			ce->frame_thickness_ = 2.f;
			ce->frame_color_ = Vec4c(20, 190, 20, 255);
			ce->clip_children = true;
		}
		ui::push_parent(ui::current_entity());

			ui::e_empty();
			ui::next_element_pos = Vec2f(0.f, -16.f * 3.8f);
			ui::next_element_size = Vec2f(16.f * board_width, 16.f * board_height);
			ui::c_element();
			{
				app.board = cTileMap::create();
				app.board->cell_size = Vec2f(16.f);
				app.board->set_size(Vec2u(board_width, board_height));
				for (auto i = 0; i < atlas->tile_count(); i++)
					app.board->add_tile((atlas->canvas_slot_ << 16) + i);
				ui::current_entity()->add_component(app.board);
			}

		ui::pop_parent();

		ui::e_text(L"");
		ui::c_aligner(AlignxLeft, AlignyBottom);
		add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
		}, new_mail_p(ui::current_entity()->get_component(cText)));

	ui::pop_parent();

	srand(time(0));

	looper().loop([](void* c) {
		(*(App**)c)->run();
	}, new_mail_p(&app));

	return 0;
}
