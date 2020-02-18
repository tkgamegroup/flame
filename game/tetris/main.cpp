#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/utils/app.h>

#include "mino.h"
#include "key.h"

using namespace flame;
using namespace graphics;

const auto board_width = 10U;
const auto board_height = 24U;
const auto down_ticks = 24U;

enum TileIndex
{
	TileGrid,
	TileMino1,
	TileMino2,
	TileMino3,
	TileMino4,
	TileMino5,
	TileMino6,
	TileMino7,
	TileGray,
	TileGhost
};

struct MyApp : App
{
	sEventDispatcher* s_event_dispatcher;
	Atlas* atlas;

	cTileMap* board_main;
	cTileMap* board_hold;
	cTileMap* board_next[6];
	cText* text_time;
	cText* text_lines;
	cText* text_score;

	bool just_down_pause;
	bool just_down_rotate_left;
	bool just_down_rotate_right;
	bool just_down_hard_drop;
	bool just_down_hold;
	int left_frames;
	int right_frames;

	float time;
	float play_time;
	uint clear_lines;
	uint score;
	bool gaming;
	Vec2i mino_pos;
	MinoType mino_type;
	MinoType mino_held;
	uint mino_rotation;
	Vec2i mino_coords[3];
	uint mino_bottom_dist;
	uint mino_ticks;
	Vec2u mino_pack_idx;
	MinoType mino_packs[2][MinoTypeCount];

	MyApp()
	{
		init_mino();
		init_key();
		gaming = false;
	}

	void create_home_scene()
	{
		ui::push_parent(root);

		ui::e_begin_layout(LayoutVertical, 8.f);
		ui::c_aligner(AlignxMiddle, AlignyMiddle);
		ui::push_style_1u(ui::FontSize, 40);
		ui::e_text(L"Tetris");
		ui::c_aligner(AlignxMiddle, AlignyFree);
		ui::pop_style(ui::FontSize);
		ui::push_style_1u(ui::FontSize, 20);
		ui::e_button(L"40 Lines RTA", [](void*) {
			looper().add_event([](void*, bool*) {
				app.root->remove_children(1, -1);
				app.create_game_scene();
				app.start_game();
			}, Mail<>());
		}, Mail<>());
		ui::c_aligner(AlignxMiddle, AlignyFree);
		ui::pop_style(ui::FontSize);
		ui::e_end_layout();

		ui::pop_parent();
	}

	void set_board_tiles(cTileMap* m)
	{
		m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH("grid.png")));
		for (auto i = 1; i <= 7; i++)
			m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH((std::to_string(i) + ".png").c_str())));
		m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH("gray.png")));
		m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH("ghost.png")));
	}

	void create_game_scene()
	{
		ui::push_parent(root);
		ui::push_style_1u(ui::FontSize, 20);

		auto block_size = 24.f;

		ui::e_empty();
		ui::next_element_pos = Vec2f(120.f, 20.f);
		ui::next_element_size = Vec2f(block_size * board_width, block_size * (board_height - 3.8f));
		{
			auto ce = ui::c_element();
			ce->frame_thickness_ = 6.f;
			ce->frame_color_ = Vec4c(255);
			ce->clip_children = true;
		}
		ui::push_parent(ui::current_entity());

		ui::e_empty();
		ui::next_element_pos = Vec2f(0.f, -block_size * 3.8f);
		ui::next_element_size = Vec2f(block_size * board_width, block_size * board_height);
		ui::c_element();
		{
			board_main = cTileMap::create();
			board_main->cell_size = Vec2f(block_size);
			board_main->set_size(Vec2u(board_width, board_height));
			board_main->clear_cells(TileGrid);
			set_board_tiles(board_main);
			ui::current_entity()->add_component(board_main);
		}

		ui::pop_parent();

		block_size = 16.f;

		ui::next_element_pos = Vec2f(35.f, 20.f);
		ui::e_text(L"Hold");

		ui::e_empty();
		ui::next_element_pos = Vec2f(20.f, 50.f);
		ui::next_element_size = Vec2f(block_size * 4 + 8.f);
		{
			auto ce = ui::c_element();
			ce->inner_padding_ = Vec4f(4.f);
			ce->color_ = Vec4c(30, 30, 30, 255);
		}
		{
			board_hold = cTileMap::create();
			board_hold->cell_size = Vec2f(block_size);
			board_hold->set_size(Vec2u(4));
			set_board_tiles(board_hold);
			ui::current_entity()->add_component(board_hold);
		}

		block_size = 12.f;

		ui::next_element_pos = Vec2f(400.f, 20.f);
		ui::e_text(L"Next");

		ui::e_empty();
		ui::next_element_pos = Vec2f(390.f, 50.f);
		ui::next_element_size = Vec2f(block_size * 4 + 8.f, 52.f * array_size(board_next) + 8.f);
		{
			auto ce = ui::c_element();
			ce->color_ = Vec4c(30, 30, 30, 255);
		}

		for (auto i = 0; i < array_size(board_next); i++)
		{
			ui::e_empty();
			ui::next_element_pos = Vec2f(390.f, 50.f + 52.f * i);
			ui::next_element_size = Vec2f(block_size * 4 + 8.f);
			ui::c_element()->inner_padding_ = Vec4f(4.f);
			{
				board_next[i] = cTileMap::create();
				board_next[i]->cell_size = Vec2f(block_size);
				board_next[i]->set_size(Vec2u(4));
				set_board_tiles(board_next[i]);
				ui::current_entity()->add_component(board_next[i]);
			}
		}

		ui::next_element_pos = Vec2f(20.f, 220.f);
		text_time = ui::e_text(L"Time: 00:00")->get_component(cText);
		ui::next_element_pos = Vec2f(20.f, 270.f);
		text_lines = ui::e_text(L"Lines: 0")->get_component(cText);
		ui::next_element_pos = Vec2f(20.f, 320.f);
		text_score = ui::e_text(L"Score: 0")->get_component(cText);

		ui::pop_style(ui::FontSize);
		ui::pop_parent();

		app.play_time = 0.f;
	}

	void add_count_down()
	{
		struct Capture
		{
			uint time;
			cText* text;
		}capture;
		capture.time = 3;
		ui::push_parent(root);
		ui::push_style_1u(ui::FontSize, 50);
		ui::next_element_pos = Vec2f(230.f, 200.f);
		capture.text = ui::e_text(L"3")->get_component(cText);
		ui::pop_style(ui::FontSize);
		ui::pop_parent();
		looper().add_event([](void* c, bool* go_on) {
			auto& capture = *(Capture*)c;
			capture.time--;
			capture.text->set_text(std::to_wstring(capture.time).c_str());
			if (capture.time == 0)
			{
				auto e = capture.text->entity;
				e->parent()->remove_child(e);
				app.gaming = true;
			}
			else
				*go_on = true;
		}, new_mail(&capture), 1.f);
	}

	void start_game()
	{
		board_main->clear_cells(TileGrid);
		board_hold->clear_cells(-1);
		for (auto i = 0; i < array_size(board_next); i++)
			board_next[i]->clear_cells(-1);
		text_time->set_text(L"Time: 00:00");
		text_lines->set_text(L"Lines: 0");
		text_score->set_text(L"Score: 0");

		just_down_pause = false;
		just_down_rotate_left = false;
		just_down_rotate_right = false;
		just_down_hard_drop = false;
		just_down_hold = false;
		left_frames = -1;
		right_frames = -1;

		time = 0.f;
		clear_lines = 0;
		score = 0;
		mino_pos = Vec2i(0, -1);
		mino_type = MinoTypeCount;
		mino_held = MinoTypeCount;
		mino_pack_idx = Vec2u(0, 0);
		mino_rotation = 0;
		mino_bottom_dist = 0;
		mino_ticks = 0;
		for (auto i = 0; i < 2; i++)
			shuffle_pack(i);

		add_count_down();
	}

	void shuffle_pack(uint idx)
	{
		auto& curr_pack = mino_packs[idx];
		for (auto i = 0; i < MinoTypeCount; i++)
			curr_pack[i] = (MinoType)i;
		for (auto i = 0; i < MinoTypeCount; i++)
			std::swap(curr_pack[i], curr_pack[rand() % MinoTypeCount]);
	}

	void toggle_board(cTileMap* board, int idx, const Vec2i& pos, uint offset_y, Vec2i* coords)
	{
		board->set_cell(Vec2u(pos) + Vec2u(0, offset_y), idx);
		board->set_cell(Vec2u(pos + coords[0] + Vec2u(0, offset_y)), idx);
		board->set_cell(Vec2u(pos + coords[1] + Vec2u(0, offset_y)), idx);
		board->set_cell(Vec2u(pos + coords[2] + Vec2u(0, offset_y)), idx);
	}

	bool check_board(const Vec2i& p)
	{
		return 
			board_main->cell(mino_pos + p) == TileGrid &&
			board_main->cell(mino_pos + p + mino_coords[0]) == TileGrid &&
			board_main->cell(mino_pos + p + mino_coords[1]) == TileGrid &&
			board_main->cell(mino_pos + p + mino_coords[2]) == TileGrid;
	}

	bool check_board(Vec2i* in, const Vec2i& p)
	{
		return
			board_main->cell(p) == TileGrid &&
			board_main->cell(in[0] + p) == TileGrid &&
			board_main->cell(in[1] + p) == TileGrid &&
			board_main->cell(in[2] + p) == TileGrid;
	}

	bool line_empty(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board_main->cell(Vec2i(x, l)) != TileGrid)
				return false;
		}
		return true;
	}

	bool line_full(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board_main->cell(Vec2i(x, l)) == TileGrid)
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

	void on_frame() override
	{
		auto& key_states = s_event_dispatcher->key_states;
		if (!just_down_pause)
			just_down_pause = key_states[key_map[KEY_PAUSE]] == (KeyStateDown | KeyStateJust);
		if (!just_down_rotate_left)
			just_down_rotate_left = key_states[key_map[KEY_ROTATE_LEFT]] == (KeyStateDown | KeyStateJust);
		if (!just_down_rotate_right)
			just_down_rotate_right = key_states[key_map[KEY_ROTATE_RIGHT]] == (KeyStateDown | KeyStateJust);
		if (!just_down_hard_drop)
			just_down_hard_drop = key_states[key_map[KEY_HARD_DROP]] == (KeyStateDown | KeyStateJust);
		if (!just_down_hold)
			just_down_hold = key_states[key_map[KEY_HARD_DROP]] == (KeyStateDown | KeyStateJust);

		auto dt = looper().delta_time;
		time += dt;
		if (gaming)
			play_time += dt;
		const auto frame_rate = 1.f / (24.f * ((key_states[Key_F1] & KeyStateDown) ? 100.f : 1.f));
		while (time > frame_rate)
		{
			if (gaming)
			{
				if (just_down_pause)
				{
					gaming = false;
					ui::e_begin_dialog(Vec4c(100));
					ui::e_text(L"Pausing");
					ui::e_begin_layout(LayoutHorizontal, 4.f);
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(Icon_HOME, [](void*) {
						ui::remove_top_layer(app.root);
						looper().add_event([](void*, bool*) {
							app.root->remove_children(1, -1);
							app.create_home_scene();
						}, Mail<>());
					}, Mail<>());
					ui::e_button(Icon_REPEAT, [](void*) {
						ui::remove_top_layer(app.root);
						app.play_time = 0.f;
						app.start_game();
					}, Mail<>());
					ui::e_button(Icon_TIMES, [](void*) {
						ui::remove_top_layer(app.root);
						app.add_count_down();
					}, Mail<>());
					ui::e_end_layout();
					ui::e_end_dialog();
				}
			}

			if (gaming)
			{
				if (mino_pos.y() != -1)
				{
					toggle_board(board_main, TileGrid, mino_pos, 0, mino_coords);
					if (mino_bottom_dist > 0)
						toggle_board(board_main, TileGrid, mino_pos, mino_bottom_dist, mino_coords);
				}

				if (mino_pos.y() < 0)
				{
					if (mino_pos.y() == -1 || mino_type == MinoTypeCount)
					{
						mino_type = mino_packs[mino_pack_idx.x()][mino_pack_idx.y()++];
						if (mino_pack_idx.y() >= MinoTypeCount)
						{
							mino_pack_idx = Vec2i(1 - mino_pack_idx.x(), 0);
							shuffle_pack(mino_pack_idx.x());
						}
						for (auto i = 0; i < array_size(board_next); i++)
						{
							board_next[i]->clear_cells();
							auto next_idx = mino_pack_idx;
							next_idx.y() += i;
							if (next_idx.y() >= MinoTypeCount)
							{
								next_idx.x() = 1 - next_idx.x();
								next_idx.y() %= MinoTypeCount;
							}
							auto t = mino_packs[next_idx.x()][next_idx.y()];
							Vec2i coords[3];
							for (auto j = 0; j < 3; j++)
								coords[j] = g_mino_coords[t][j];
							toggle_board(board_next[i], TileMino1 + t, Vec2i(1), 0, coords);
						}
					}
					if (mino_pos.y() == -2)
					{
						board_hold->clear_cells();
						if (mino_held != MinoTypeCount)
						{
							Vec2i coords[3];
							for (auto i = 0; i < 3; i++)
								coords[i] = g_mino_coords[mino_held][i];
							toggle_board(board_hold, TileMino1 + mino_held, Vec2i(1), 0, coords);
						}
					}
					mino_pos = Vec2i(4, 3);
					mino_rotation = 0;
					for (auto i = 0 ; i < 3; i++)
						mino_coords[i] = g_mino_coords[mino_type][i];
					mino_ticks = 0;
				}

				if (just_down_hold)
				{
					mino_pos.y() = -2;
					std::swap(mino_held, mino_type);
				}
				else
				{
					auto r = 0;
					if (just_down_rotate_left)
						r--;
					if (just_down_rotate_right)
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
					if (key_states[key_map[KEY_LEFT]] & KeyStateDown)
					{
						if (left_frames == -1)
							left_frames = 0;
						else
							left_frames++;
						if (left_frames == 0 || (left_frames >= 6 && left_frames % 2 == 0))
							mx--;
					}
					else
						left_frames = -1;
					if (key_states[key_map[KEY_RIGHT]] & KeyStateDown)
					{
						if (right_frames == -1)
							right_frames = 0;
						else
							right_frames++;
						if (right_frames == 0 || (right_frames >= 6 && right_frames % 2 == 0))
							mx++;
					}
					else
						right_frames = -1;
					if (mx != 0 && check_board(Vec2i(mx, 0)))
					{
						mino_pos.x() += mx;
						mino_ticks = 0;
					}

					mino_bottom_dist = 0;
					while (check_board(Vec2i(0, mino_bottom_dist + 1)))
						mino_bottom_dist++;
					auto down_ticks_final = down_ticks;
					if (mino_bottom_dist == 0)
						down_ticks_final = 12;
					else if (key_states[key_map[KEY_SOFT_DROP]] & KeyStateDown)
						down_ticks_final = 1;
					if (just_down_hard_drop || mino_ticks >= down_ticks_final)
					{
						if (just_down_hard_drop || mino_bottom_dist == 0)
						{
							mino_pos.y() += mino_bottom_dist;
							mino_bottom_dist = 0;
							toggle_board(board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
							for (auto i = (int)board_height - 1; i >= 0; i--)
							{
								if (line_full(i))
								{
									for (auto x = 0; x < board_width; x++)
										board_main->set_cell(Vec2u(x, i), TileGrid);
									for (auto j = i; j > 0; j--)
									{
										for (auto x = 0; x < board_width; x++)
											board_main->set_cell(Vec2u(x, j), board_main->cell(Vec2i(x, j - 1)));
									}
									i++;
									clear_lines++;
								}
							}
							if (!line_empty(3) || clear_lines >= 40)
							{
								gaming = false;
								ui::e_begin_dialog(Vec4c(100));
								ui::e_text(L"Game Over");
								ui::e_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
								ui::e_text((L"Lines: " + wfmt(L"%d", clear_lines)).c_str());
								ui::e_text((L"Score: " + wfmt(L"%d", score)).c_str());
								ui::e_begin_layout(LayoutHorizontal, 4.f);
								ui::c_aligner(AlignxMiddle, AlignyFree);
								ui::e_button(Icon_HOME, [](void*) {
									ui::remove_top_layer(app.root);
									looper().add_event([](void*, bool*) {
										app.root->remove_children(1, -1);
										app.create_home_scene();
									}, Mail<>());
								}, Mail<>());
								ui::e_button(Icon_REPEAT, [](void*) {
									ui::remove_top_layer(app.root);
									app.play_time = 0.f;
									app.start_game();
								}, Mail<>());
								ui::e_end_layout();
								ui::e_end_dialog();
							}
							mino_pos.y() = -1;
						}
						else
						{
							mino_pos.y()++;
							mino_bottom_dist--;
						}
						mino_ticks = 0;
					}
					mino_ticks++;

					if (mino_pos.y() != -1)
					{
						if (mino_bottom_dist)
							toggle_board(board_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords);
						toggle_board(board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
					}
				}

				text_time->set_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
				text_lines->set_text((L"Lines: " + std::to_wstring(clear_lines)).c_str());
				text_score->set_text((L"Score: " + std::to_wstring(score)).c_str());
			}

			just_down_pause = false;
			just_down_rotate_left = false;
			just_down_rotate_right = false;
			just_down_hard_drop = false;
			just_down_hold = false;

			time -= frame_rate;
		}
	}
}app;

int main(int argc, char **args)
{
	app.create("Tetris", Vec2u(500, 550), WindowFrame);

	auto w = app.u->world(0);

	app.s_event_dispatcher = w->get_system(sEventDispatcher);

	app.atlas = Atlas::load(app.d, L"../game/tetris/art/atlas/main.png");
	app.canvas->add_atlas(app.atlas);

	auto root = w->root();
	app.root = root;
	ui::set_current_entity(root);
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::set_current_root(root);

	ui::push_parent(app.root);
	ui::e_text(L"");
	ui::c_aligner(AlignxLeft, AlignyBottom);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, new_mail_p(ui::current_entity()->get_component(cText)));
	ui::pop_parent();

	srand(time(0));

	app.create_home_scene();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
