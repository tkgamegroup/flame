#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/network/network.h>
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

enum GameMode
{
	GameSingleMarathon,
	GameSingleRTA,
	GameSinglePractice
};

enum RoomState
{
	RoomPrepare,
	RoomGaming
};

struct Player
{
	std::wstring name;
	void* client_id;
};

struct MyApp : App
{
	std::wstring your_name;
	Server* server;
	std::wstring room_name;
	RoomState room_state;
	std::vector<Player> players;
	Client* client;

	sEventDispatcher* s_event_dispatcher;
	Atlas* atlas;

	Entity* e_base;
	cTileMap* c_board_main;
	cTileMap* c_board_hold;
	cTileMap* c_board_next[6];
	cText* c_text_time;
	cText* c_text_level;
	cText* c_text_lines;
	cText* c_text_score;

	bool just_down_rotate_left;
	bool just_down_rotate_right;
	bool just_down_hard_drop;
	bool just_down_hold;
	int left_frames;
	int right_frames;

	float time;
	float play_time;
	uint level;
	uint clear_lines;
	uint score;
	bool paused;
	bool running;
	GameMode game_mode;
	Vec2i mino_pos;
	MinoType mino_type;
	MinoType mino_hold;
	bool mino_just_hold;
	uint mino_rotation;
	Vec2i mino_coords[3];
	int mino_reset_times;
	uint mino_bottom_dist;
	uint mino_ticks;
	Vec2u mino_pack_idx;
	MinoType mino_packs[2][MinoTypeCount];

	MyApp()
	{
		server = nullptr;
		client = nullptr;

		init_mino();
		init_key();
	}

	~MyApp()
	{
		std::ofstream user_data(L"user_data.txt");
		user_data << w2s(your_name) << "\n";
		user_data.close();
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
				ui::e_button(L"Single", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_single_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Online", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_online_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_single_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				ui::e_button(L"Marathon", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.game_mode = GameSingleMarathon;
						app.create_game_scene();
						app.start_game();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"RTA 40", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.game_mode = GameSingleRTA;
						app.create_game_scene();
						app.start_game();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Practice", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_single_pratice_otions_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_home_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_single_pratice_otions_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				ui::e_button(L"Start", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.game_mode = GameSinglePractice;
						app.create_game_scene();
						app.start_game();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_single_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_online_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
				ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				ui::e_button(L"Local", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_online_local_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"www.not-yet.com", [](void*) {
					looper().add_event([](void*, bool*) {

					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_home_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_online_local_scene()
	{
		ui::push_parent(root);
		ui::next_element_size = Vec2f(500.f, 0.f);
		ui::e_begin_layout(LayoutVertical, 8.f, false, false)->get_component(cElement)->inner_padding_ = 8.f;
		ui::c_aligner(SizeFixed, SizeFitParent)->x_align_ = AlignxMiddle;
			ui::push_style_1u(ui::FontSize, 20);
			ui::e_begin_layout(LayoutHorizontal, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_text(L"Your Name");
				ui::e_edit(300.f, app.your_name.c_str())->get_component(cText)->data_changed_listeners.add([](void*, Component* c, uint hash, void*) {
					if (hash == FLAME_CHASH("text"))
						app.your_name = ((cText*)c)->text();
				}, Mail<>());
			ui::e_end_layout();
			ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f), 4.f, 2.f);
				auto e_room_list = ui::e_begin_list(true);
				ui::e_end_list();
			ui::e_end_scroll_view1();
			ui::e_begin_layout(LayoutHorizontal, 8.f)->get_component(cLayout)->fence = 3;
			ui::c_aligner(SizeFitParent, SizeFixed);
				ui::e_button(Icon_REFRESH, [](void* c) {
					auto e_room_list = *(Entity**)c;
					looper().add_event([](void* c, bool*) {
						auto e_room_list = *(Entity**)c;
						e_room_list->remove_children(0, -1);
						nlohmann::json req;
						req["action"] = "get_room";
						auto str = req.dump();
						board_cast(2434, str.data(), str.size(), 1, [](void* c, const char* ip, const char* msg, uint size) {
							auto e_room_list = *(Entity**)c;
							auto reply = nlohmann::json::parse(std::string(msg, size));
							struct Capture
							{
								Entity* e_room_list;
								std::wstring name;
								std::wstring host;
							}capture;
							capture.e_room_list = e_room_list;
							capture.name = s2w(reply["name"].get<std::string>());
							capture.host = s2w(reply["host"].get<std::string>());
							looper().add_event([](void* c, bool*) {
								auto& capture = *(Capture*)c;
								ui::push_parent(capture.e_room_list);
								ui::e_list_item((L"Name:" + capture.name + L" Creator:" + capture.host).c_str());
								ui::pop_parent();
							}, new_mail(&capture));
						}, new_mail_p(e_room_list));
					}, new_mail_p(e_room_list));
				}, new_mail_p(e_room_list))->get_component(cEventReceiver)->on_mouse(KeyStateDown | KeyStateUp, Mouse_Null, Vec2i(0));
				ui::e_button(L"Create Room", [](void*) {
					if (app.your_name.empty())
						ui::e_message_dialog(L"Your Name Cannot Not Be Empty!");
					else
					{
						ui::e_input_dialog(L"Room Name", [](void*, bool ok, const wchar_t* text) {
							if (ok && text[0])
							{
								app.room_name = text;
								app.room_state = RoomPrepare;
								app.server = Server::create(SocketNormal, 2434, 
								[](void*, void* id, const char* msg, uint size) {
									auto req = nlohmann::json::parse(std::string(msg, size));
									if (req["action"] == "get_room")
									{
										nlohmann::json reply;
										reply["name"] = w2s(app.room_name);
										reply["host"] = w2s(app.your_name);
										auto str = reply.dump();
										app.server->send(id, str.data(), str.size(), true);
									}
								},
								[](void*, void* id) {
									if (app.room_state == RoomPrepare && app.players.size())
									{
										;
									}
								}, Mail<>());
								looper().add_event([](void*, bool*) {
									app.root->remove_children(1, -1);

								}, Mail<>());
							}
						}, Mail<>());
					}
				}, Mail<>());
				ui::e_button(L"Join Room", [](void* c) {
					auto e_room_list = *(Entity**)c;
					auto selected = e_room_list->get_component(cList)->selected;
					if (selected)
					{
						if (app.your_name.empty())
							ui::e_message_dialog(L"Your Name Cannot Not Be Empty!");
						else
						{

						}
					}
				}, new_mail_p(e_room_list));
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_online_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxRight, AlignyTop);
			ui::e_end_layout();
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

			ui::next_element_pos = Vec2f(100.f, 0.f);
			e_base = ui::e_element();
			ui::push_parent(e_base);

				auto block_size = 24.f;

				ui::e_empty();
				ui::next_element_pos = Vec2f(120.f, 50.f);
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
						c_board_main = cTileMap::create();
						c_board_main->cell_size = Vec2f(block_size);
						c_board_main->set_size(Vec2u(board_width, board_height));
						c_board_main->clear_cells(TileGrid);
						set_board_tiles(c_board_main);
						ui::current_entity()->add_component(c_board_main);
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
					c_board_hold = cTileMap::create();
					c_board_hold->cell_size = Vec2f(block_size);
					c_board_hold->set_size(Vec2u(4));
					set_board_tiles(c_board_hold);
					ui::current_entity()->add_component(c_board_hold);
				}

				block_size = 12.f;

				ui::next_element_pos = Vec2f(400.f, 20.f);
				ui::e_text(L"Next");

				ui::e_empty();
				ui::next_element_pos = Vec2f(390.f, 50.f);
				ui::next_element_size = Vec2f(block_size * 4 + 8.f, 52.f * array_size(c_board_next) + 8.f);
				{
					auto ce = ui::c_element();
					ce->color_ = Vec4c(30, 30, 30, 255);
				}
				for (auto i = 0; i < array_size(c_board_next); i++)
				{
					ui::e_empty();
					ui::next_element_pos = Vec2f(390.f, 50.f + 52.f * i);
					ui::next_element_size = Vec2f(block_size * 4 + 8.f);
					ui::c_element()->inner_padding_ = Vec4f(4.f);
					{
						c_board_next[i] = cTileMap::create();
						c_board_next[i]->cell_size = Vec2f(block_size);
						c_board_next[i]->set_size(Vec2u(4));
						set_board_tiles(c_board_next[i]);
						ui::current_entity()->add_component(c_board_next[i]);
					}
				}

				{
					auto pos = Vec2f(500.f, 220.f);
					ui::next_element_pos = pos; pos.y() += 50.f;
					c_text_time = ui::e_text(L"")->get_component(cText);
					if (is_one_of(game_mode, { GameSingleMarathon, GameSinglePractice }))
					{
						ui::next_element_pos = pos; pos.y() += 50.f;
						c_text_level = ui::e_text(L"")->get_component(cText);
					}
					ui::next_element_pos = pos; pos.y() += 50.f;
					c_text_lines = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = pos; pos.y() += 50.f;
					c_text_score = ui::e_text(L"")->get_component(cText);
				}

			ui::pop_parent();

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

			~Capture()
			{
				auto e = text->entity;
				e->parent()->remove_child(e);
				app.running = true;
			}
		};
		auto capture = new_mail<Capture>();
		capture.p->time = 3;
		ui::push_parent(e_base);
		ui::push_style_1u(ui::FontSize, 50);
		ui::next_element_pos = Vec2f(230.f, 200.f);
		capture.p->text = ui::e_text(L"3")->get_component(cText);
		ui::pop_style(ui::FontSize);
		ui::pop_parent();
		looper().add_event([](void* c, bool* go_on) {
			auto& capture = *(Capture*)c;
			capture.time--;
			capture.text->set_text(std::to_wstring(capture.time).c_str());
			if (capture.time > 0)
				*go_on = true;
		}, capture, 1.f, FLAME_CHASH("count_down"));
	}

	void update_status()
	{
		c_text_time->set_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
		if (c_text_level)
			c_text_level->set_text((L"Level: " + std::to_wstring(level)).c_str());
		if (game_mode == GameSingleRTA)
			c_text_lines->set_text((L"Left: " + std::to_wstring(max(0, 40 - (int)clear_lines))).c_str());
		else
			c_text_lines->set_text((L"Lines: " + std::to_wstring(clear_lines)).c_str());
		c_text_score->set_text((L"Score: " + std::to_wstring(score)).c_str());
	}

	void start_game()
	{
		c_board_main->clear_cells(TileGrid);
		c_board_hold->clear_cells(-1);
		for (auto i = 0; i < array_size(c_board_next); i++)
			c_board_next[i]->clear_cells(-1);

		just_down_rotate_left = false;
		just_down_rotate_right = false;
		just_down_hard_drop = false;
		just_down_hold = false;
		left_frames = -1;
		right_frames = -1;

		time = 0.f;
		level = 1;
		clear_lines = 0;
		score = 0;
		mino_pos = Vec2i(0, -1);
		mino_type = MinoTypeCount;
		mino_hold = MinoTypeCount;
		mino_just_hold = false;
		mino_pack_idx = Vec2u(0, 0);
		mino_rotation = 0;
		mino_reset_times = -1;
		mino_bottom_dist = 0;
		mino_ticks = 0;
		for (auto i = 0; i < 2; i++)
			shuffle_pack(i);

		update_status();

		paused = false;
		running = false;
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
			c_board_main->cell(mino_pos + p) == TileGrid &&
			c_board_main->cell(mino_pos + p + mino_coords[0]) == TileGrid &&
			c_board_main->cell(mino_pos + p + mino_coords[1]) == TileGrid &&
			c_board_main->cell(mino_pos + p + mino_coords[2]) == TileGrid;
	}

	bool check_board(Vec2i* in, const Vec2i& p)
	{
		return
			c_board_main->cell(p) == TileGrid &&
			c_board_main->cell(in[0] + p) == TileGrid &&
			c_board_main->cell(in[1] + p) == TileGrid &&
			c_board_main->cell(in[2] + p) == TileGrid;
	}

	bool line_empty(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (c_board_main->cell(Vec2i(x, l)) != TileGrid)
				return false;
		}
		return true;
	}

	bool line_full(uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (c_board_main->cell(Vec2i(x, l)) == TileGrid)
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
		if (!just_down_rotate_left)
			just_down_rotate_left = key_states[key_map[KEY_ROTATE_LEFT]] == (KeyStateDown | KeyStateJust);
		if (!just_down_rotate_right)
			just_down_rotate_right = key_states[key_map[KEY_ROTATE_RIGHT]] == (KeyStateDown | KeyStateJust);
		if (!just_down_hard_drop)
			just_down_hard_drop = key_states[key_map[KEY_HARD_DROP]] == (KeyStateDown | KeyStateJust);
		if (!just_down_hold)
			just_down_hold = key_states[key_map[KEY_HOLD]] == (KeyStateDown | KeyStateJust);

		if (key_states[key_map[KEY_PAUSE]] == (KeyStateDown | KeyStateJust))
		{
			if (!paused)
			{
				looper().clear_events(FLAME_CHASH("count_down"));

				ui::e_begin_dialog();
				ui::e_text(L"Paused");
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Resume", [](void*) {
					app.paused = false;
					ui::remove_top_layer(app.root);
					app.running = false;
					app.add_count_down();

				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Restart", [](void*) {
					app.paused = false;
					ui::remove_top_layer(app.root);
					app.play_time = 0.f;
					app.start_game();

				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Quit", [](void*) {
					app.paused = false;
					ui::remove_top_layer(app.root);
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_home_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_end_dialog();

				paused = true;
			}
			else
			{
				paused = false;
				ui::remove_top_layer(app.root);
				running = false;
				add_count_down();
			}
		}

		auto dt = looper().delta_time;
		time += dt;
		if (!paused && running)
			play_time += dt;
		const auto frame_rate = 1.f / 24.f;
		while (time > frame_rate)
		{
			if (!paused && running)
			{
				if (mino_pos.y() != -1)
				{
					toggle_board(c_board_main, TileGrid, mino_pos, 0, mino_coords);
					if (mino_bottom_dist > 0)
						toggle_board(c_board_main, TileGrid, mino_pos, mino_bottom_dist, mino_coords);
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
						for (auto i = 0; i < array_size(c_board_next); i++)
						{
							c_board_next[i]->clear_cells();
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
							toggle_board(c_board_next[i], TileMino1 + t, Vec2i(1), 0, coords);
						}
					}
					if (mino_pos.y() == -2)
					{
						c_board_hold->clear_cells();
						if (mino_hold != MinoTypeCount)
						{
							Vec2i coords[3];
							for (auto i = 0; i < 3; i++)
								coords[i] = g_mino_coords[mino_hold][i];
							toggle_board(c_board_hold, TileMino1 + mino_hold, Vec2i(1), 0, coords);
						}
					}
					mino_pos = Vec2i(4, 3);
					mino_rotation = 0;
					for (auto i = 0 ; i < 3; i++)
						mino_coords[i] = g_mino_coords[mino_type][i];
					mino_reset_times = -1;
					mino_ticks = 0;
				}

				if (just_down_hold && (game_mode == GameSinglePractice || mino_just_hold == false))
				{
					mino_pos.y() = -2;
					std::swap(mino_hold, mino_type);
					mino_just_hold = true;
				}
				else
				{
					auto moved = false;

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
							moved = true;
						}
					}

					auto mx = 0;
					if (key_states[key_map[KEY_LEFT]] & KeyStateDown)
					{
						if (left_frames == -1)
							left_frames = 0;
						else
							left_frames++;
						if (left_frames == 0 || (left_frames >= 5))
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
						if (right_frames == 0 || (right_frames >= 5))
							mx++;
					}
					else
						right_frames = -1;
					if (mx != 0 && check_board(Vec2i(mx, 0)))
					{
						mino_pos.x() += mx;
						moved = true;
					}

					mino_bottom_dist = 0;
					while (check_board(Vec2i(0, mino_bottom_dist + 1)))
						mino_bottom_dist++;
					if (moved)
					{
						if (game_mode == GameSinglePractice)
							mino_ticks = 0;
						else
						{
							if (mino_reset_times == -1 && mino_bottom_dist == 0)
								mino_reset_times = 0;
							if (mino_reset_times >= 0)
							{
								if (mino_reset_times >= 15)
									mino_ticks = down_ticks;
								else
									mino_ticks = 0;
								mino_reset_times++;
							}
						}
					}
					auto is_soft_drop = key_states[key_map[KEY_SOFT_DROP]] & KeyStateDown;
					auto down_ticks_final = down_ticks;
					if (is_one_of(game_mode, { GameSingleRTA, GameSinglePractice }))
						down_ticks_final = 9999;
					else
						down_ticks_final = down_ticks_final - level + 1;
					if (mino_bottom_dist == 0)
						down_ticks_final = 12;
					else if (is_soft_drop)
						down_ticks_final = 1;
					if (just_down_hard_drop || mino_ticks >= down_ticks_final)
					{
						if (just_down_hard_drop || mino_bottom_dist == 0)
						{
							mino_pos.y() += mino_bottom_dist;
							if (just_down_hard_drop)
								score += mino_bottom_dist * 2;
							mino_bottom_dist = 0;
							toggle_board(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
							auto full_lines = 0;
							for (auto i = (int)board_height - 1; i >= 0; i--)
							{
								if (line_full(i))
								{
									for (auto x = 0; x < board_width; x++)
										c_board_main->set_cell(Vec2u(x, i), TileGrid);
									for (auto j = i; j > 0; j--)
									{
										for (auto x = 0; x < board_width; x++)
											c_board_main->set_cell(Vec2u(x, j), c_board_main->cell(Vec2i(x, j - 1)));
									}
									i++;
									full_lines++;
								}
							}
							clear_lines += full_lines;
							if (game_mode == GameSingleMarathon && full_lines > 0 && clear_lines % 5 == 0)
							{
								level++;
								level = min(24U, level);
							}
							switch (full_lines)
							{
							case 1:
								score += 100;
								break;
							case 2:
								score += 300;
								break;
							case 3:
								score += 500;
								break;
							case 4:
								score += 800;
								break;
							}
							auto gameover = !line_empty(3);
							if (!gameover)
							{
								if (game_mode == GameSingleRTA && clear_lines >= 40)
									gameover = true;
							}
							if (gameover)
							{
								running = false;
								ui::e_begin_dialog();
									ui::e_text(L"Game Over");
									ui::c_aligner(AlignxMiddle, AlignyFree);
									ui::e_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
									ui::e_text((L"Level: " + wfmt(L"%d", level)).c_str());
									ui::e_text((L"Lines: " + wfmt(L"%d", clear_lines)).c_str());
									ui::e_text((L"Score: " + wfmt(L"%d", score)).c_str());
									ui::e_button(L"Quit", [](void*) {
										ui::remove_top_layer(app.root);
										looper().add_event([](void*, bool*) {
											app.root->remove_children(1, -1);
											app.create_home_scene();
										}, Mail<>());
									}, Mail<>());
									ui::c_aligner(AlignxMiddle, AlignyFree);
									ui::e_button(L"Restart", [](void*) {
										ui::remove_top_layer(app.root);
										app.play_time = 0.f;
										app.start_game();
									}, Mail<>());
									ui::c_aligner(AlignxMiddle, AlignyFree);
								ui::e_end_dialog();
							}
							mino_pos.y() = -1;
							mino_just_hold = false;
						}
						else
						{
							mino_pos.y()++;
							mino_bottom_dist--;
							if (is_soft_drop)
								score++;
						}
						mino_ticks = 0;
					}
					mino_ticks++;

					if (mino_pos.y() != -1)
					{
						if (mino_bottom_dist)
							toggle_board(c_board_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords);
						toggle_board(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
					}
				}

				update_status();
			}

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
	app.create("Tetris", Vec2u(800, 600), WindowFrame);

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

	std::ifstream user_data(L"user_data.txt");
	if (user_data.good())
	{
		std::string line;
		std::getline(user_data, line);
		app.your_name = s2w(line);
		user_data.close();
	}

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
