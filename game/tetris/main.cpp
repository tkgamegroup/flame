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

#include "mino.h"
#include "key.h"

using namespace flame;
using namespace graphics;

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

	bool just_down_key[KEY_COUNT];

	float time;
	bool gaming;
	Vec2i mino_pos;
	MinoType mino_type;
	uint mino_rotation;
	Vec2i mino_coords[3];
	uint mino_bottom_dist;
	uint mino_ticks;

	App()
	{
		init_mino();
		init_key();

		for (auto i = 0; i < KEY_COUNT; i++)
			just_down_key[i] = false;

		time = 0.f;
		gaming = true;
		mino_pos.x() = 0;
		mino_pos.y() = -1;
		mino_rotation = 0;
		mino_bottom_dist = 0;
		mino_ticks = 0;
	}

	void toggle_board(int idx, uint offset_y)
	{
		board->set_cell(Vec2u(mino_pos) + Vec2u(0, offset_y), idx);
		board->set_cell(Vec2u(mino_pos + mino_coords[0] + Vec2u(0, offset_y)), idx);
		board->set_cell(Vec2u(mino_pos + mino_coords[1] + Vec2u(0, offset_y)), idx);
		board->set_cell(Vec2u(mino_pos + mino_coords[2] + Vec2u(0, offset_y)), idx);
	}

	bool check_board(const Vec2i& p)
	{
		return 
			board->cell(mino_pos + p) == -1 &&
			board->cell(mino_pos + p + mino_coords[0]) == -1 &&
			board->cell(mino_pos + p + mino_coords[1]) == -1 &&
			board->cell(mino_pos + p + mino_coords[2]) == -1;
	}

	bool check_board(Vec2i* in, const Vec2i& p)
	{
		return
			board->cell(p) == -1 &&
			board->cell(in[0] + p) == -1 &&
			board->cell(in[1] + p) == -1 &&
			board->cell(in[2] + p) == -1;
	}

	bool line_empty(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2i(x, l)) != -1)
				return false;
		}
		return true;
	}

	bool line_full(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2i(x, l)) == -1)
				return false;
		}
		return true;
	}

	uint get_rotation_idx(bool clockwise)
	{
		if (clockwise)
			return mino_rotation == 3 ? 0 : mino_rotation + 1;
		return mino_rotation == 0 ? 3 : mino_rotation - 1;
	}

	bool super_rotation(bool clockwise, Vec2i* out_coord, Vec2i* offset)
	{
		Mat2x2i mats[] = {
			Mat2x2i(Vec2i(0, -1), Vec2i(1, 0)),
			Mat2x2i(Vec2i(0, 1), Vec2i(-1, 0))
		};
		auto& mat = mats[clockwise ? 1 : 0];
		out_coord[0] = mat * mino_coords[0];
		out_coord[1] = mat * mino_coords[1];
		out_coord[2] = mat * mino_coords[2];
		auto offsets = g_mino_LTSZJ_offsets;
		if (mino_type == Mino_O)
			offsets = g_mino_O_offsets;
		else if (mino_type == Mino_I)
			offsets = g_mino_I_offsets;
		auto new_ridx = get_rotation_idx(clockwise);
		for (auto i = 0; i < 5; i++)
		{
			*offset = offsets[i][mino_rotation] - offsets[i][new_ridx];
			if (check_board(out_coord, mino_pos + *offset))
				return true;
		}
		return false;
	}

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		auto key_states = s_event_dispatcher->key_states;
		for (auto i = 0; i < KEY_COUNT; i++)
		{
			if (!just_down_key[i])
				just_down_key[i] = key_states[key_map[i]] == (KeyStateDown | KeyStateJust);
		}

		time += looper().delta_time;
		const auto frame_rate = 1.f / (24.f * ((key_states[Key_F1] & KeyStateDown) ? 100.f : 1.f));
		while (time > frame_rate)
		{
			if (gaming)
			{
				if (mino_pos.y() != -1)
				{
					toggle_board(-1, 0);
					if (mino_bottom_dist > 0)
						toggle_board(-1, mino_bottom_dist);
				}

				if (mino_pos.y() == -1)
				{
					mino_pos = Vec2i(4, 3);
					mino_type = MinoType(rand() % MinoTypeCount);
					mino_rotation = 0;
					for (auto i = 0 ; i < 3; i++)
						mino_coords[i] = g_mino_coords[mino_type][i];
					mino_ticks = 0;
				}

				auto r = 0;
				if (just_down_key[KEY_ROTATE_LEFT])
					r--;
				if (just_down_key[KEY_ROTATE_RIGHT])
					r++;
				if (r != 0)
				{
					Vec2i new_coords[3];
					Vec2i offset;
					if (super_rotation(r == 1, new_coords, &offset))
					{
						mino_rotation = get_rotation_idx(true);
						mino_pos += offset;
						for (auto i = 0; i < 3; i++)
							mino_coords[i] = new_coords[i];
						mino_ticks = 0;
					}
				}

				auto mx = 0;
				if (just_down_key[KEY_LEFT])
					mx--;
				if (just_down_key[KEY_RIGHT])
					mx++;
				if (mx != 0 && check_board(Vec2i(mx, 0)))
				{
					mino_pos.x() += mx;
					mino_ticks = 0;
				}

				mino_bottom_dist = 0;
				while (check_board(Vec2i(0, mino_bottom_dist + 1)))
					mino_bottom_dist++;
				auto hard_drop = just_down_key[KEY_HARD_DROP];
				if (hard_drop || mino_ticks >= (mino_bottom_dist > 0 && (key_states[key_map[KEY_SOFT_DROP]] & KeyStateDown) ? 2 : down_ticks))
				{
					if (hard_drop || mino_bottom_dist == 0)
					{
						mino_pos.y() += mino_bottom_dist;
						mino_bottom_dist = 0;
						toggle_board(mino_type, 0);
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
							ui::c_aligner(AlignxMiddle, AlignyFree);
							ui::e_end_dialog();
						}
						mino_pos.y() = -1;
					}
					else
					{
						mino_pos.y() += 1;
						mino_bottom_dist--;
					}
					mino_ticks = 0;
				}
				mino_ticks++;

				if (mino_pos.y() != -1)
				{
					toggle_board(mino_type, 0);
					if (mino_bottom_dist)
						toggle_board(MinoTypeCount + 1, mino_bottom_dist);
				}
			}

			for (auto i = 0; i < KEY_COUNT; i++)
				just_down_key[i] = false;

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
