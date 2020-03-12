#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/components/data_keeper.h>
#include <flame/universe/ui/utils.h>
#include <flame/universe/ui/typeinfo_utils.h>
#include <flame/network/network.h>
#include <flame/utils/app.h>

#include "mino.h"
#include "key.h"
#include "score.h"

using namespace flame;

const auto board_width = 10U;
const auto board_height = 24U;
const auto DOWN_TICKS = 60U;
const auto CLEAR_TICKS = 15U;
const auto mino_col_decay = Vec4c(200, 200, 200, 255);

const auto sound_move_volumn = 1.f;
const auto sound_soft_drop_volumn = 0.7f;
const auto sound_hard_drop_volumn = 0.7f;
const auto sound_clear_volumn = 0.5f;
const auto sound_hold_volumn = 0.5f;

auto fx_volumn = 10U;

auto left_right_sensitiveness = 10U;
auto left_right_speed = 2U;
auto soft_drop_speed = 1U;

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
	GameSinglePractice,
	GameMulti
};

struct Player
{
	void* id;
	std::wstring name;
	bool disconnected;
	bool ready;
	bool dead;

	Entity* e;
	cText* c_name;
	cTileMap* c_main;
	cTileMap* c_hold;
	cTileMap* c_next[6];
	cText* c_count_down;
	cText* c_ready;
	cText* c_rank;
	Entity* e_garbage;
	Entity* e_kick;

	void reset()
	{
		id = nullptr;
		disconnected = false;
		ready = false;
		dead = false;
	}
};

struct MyApp : App
{
	graphics::Atlas* atlas;

	sound::Buffer* sound_move_buf;
	sound::Buffer* sound_soft_drop_buf;
	sound::Buffer* sound_hard_drop_buf;
	sound::Buffer* sound_clear_buf;
	sound::Buffer* sound_hold_buf;
	sound::Source* sound_move_src;
	sound::Source* sound_soft_drop_src;
	sound::Source* sound_hard_drop_src;
	sound::Source* sound_clear_src;
	sound::Source* sound_hold_src;

	std::wstring my_name;
	std::wstring room_name;
	std::vector<Player> players;
	uint my_room_index;
	bool room_gaming;
	Server* server;
	uint room_max_people;
	Client* client;

	GameMode game_mode;

	cText* c_text_time;
	cText* c_text_level;
	cText* c_text_lines;
	cText* c_text_score;
	cText* c_text_special;
	Entity* e_start_or_ready;

	int left_frames;
	int right_frames;

	bool gaming;
	float play_time;
	uint level;
	uint lines;
	uint score;
	int clear_ticks;
	uint full_lines[4];
	uint combo;
	bool back_to_back;
	std::vector<uint> garbages;
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
		players.resize(1);
		players[0].id = (void*)0xffff;
		my_room_index = 0;
		server = nullptr;
		client = nullptr;

		init_mino();
		init_key();
	}

	~MyApp()
	{
		std::ofstream user_data(L"user_data.ini");
		user_data << "name = " << w2s(my_name) << "\n";
		user_data << "\n[key]\n";
		auto key_info = find_enum(FLAME_CHASH("flame::Key"));
		for (auto i = 0; i < KEY_COUNT; i++)
			user_data << w2s(key_names[i]) << " = " << key_info->find_item(key_map[i])->name() << "\n";
		user_data << "\n[sound]\n";
		user_data << "fx_volumn = " << fx_volumn << "\n";
		user_data << "\n[sensitiveness]\n";
		user_data << "left_right_sensitiveness = " << left_right_sensitiveness << "\n";
		user_data << "left_right_speed = " << left_right_speed << "\n";
		user_data << "soft_drop_speed = " << soft_drop_speed << "\n";
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
				ui::e_button(L"Marathon", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.game_mode = GameSingleMarathon;
						app.create_game_scene();
						app.start_game();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"RTA", [](void*) {
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
						app.game_mode = GameSinglePractice;
						app.create_game_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"LAN", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_lan_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Config", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_config_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_player_controls(int player_index)
	{
		auto scale = (player_index == my_room_index || room_max_people == 2) ? 1.f : 0.5f;
		auto block_size = 24U * scale;

		auto pos = Vec2f(game_mode != GameMulti ? 120.f : 0.f, 0.f);
		if (player_index != my_room_index)
		{
			switch (room_max_people)
			{
			case 2:
				pos = Vec2f(420.f, 0.f);
				break;
			case 7:
			{
				auto index = player_index;
				if (index > my_room_index)
					index--;
				pos = Vec2f(330.f + (index % 3) * 128.f, (index / 3) * 265.f);
			}
				break;
			}
		}

		auto& p = players[player_index];

		ui::next_element_pos = pos + Vec2f(80.f, 40.f) * scale;
		ui::e_begin_layout(LayoutHorizontal, 4.f);
			ui::push_style_1u(ui::FontSize, 30 * scale);
			p.c_name = ui::e_text([p]() {
				switch (app.game_mode)
				{
				case GameSingleMarathon:
					return L"Marathon";
				case GameSingleRTA:
					return L"RTA";
				case GameSinglePractice:
					return L"Practice";
				case GameMulti:
					return p.name.c_str();
				}
			}())->get_component(cText);
			if (game_mode == GameMulti && player_index == 0)
				p.c_name->color = Vec4c(91, 82, 119, 255);
			ui::pop_style(ui::FontSize);

			if (my_room_index == 0 && player_index != my_room_index)
			{
				p.e_kick = ui::e_button(Icon_TIMES, [](void* c) {
					auto index = *(int*)c;

					app.process_player_left(index);

					{
						nlohmann::json rep;
						rep["action"] = "player_left";
						rep["index"] = index;
						auto str = rep.dump();
						for (auto i = 1; i < app.players.size(); i++)
						{
							if (i != index)
							{
								auto& p = app.players[i];
								if (p.id && !p.disconnected)
									app.server->send(p.id, str.data(), str.size(), false);
							}
						}
					}
				}, new_mail(&player_index));
			}
		ui::e_end_layout();

		ui::e_empty();
		ui::next_element_pos = pos + Vec2f(85.f, 80.f) * scale;
		ui::next_element_size = Vec2f(block_size * board_width, block_size * (board_height - 3.8f));
		{
			auto ce = ui::c_element();
			ce->frame_thickness_ = 6.f * scale;
			ce->color_ = Vec4c(30, 30, 30, 255);
			ce->frame_color_ = Vec4c(255);
			ce->clip_children = true;
		}

		ui::push_parent(ui::current_entity());
			ui::e_empty();
			ui::next_element_pos = Vec2f(0.f, -block_size * 3.8f);
			ui::next_element_size = Vec2f(block_size * board_width, block_size * board_height);
			ui::c_element();
			p.c_main = cTileMap::create();
			p.c_main->cell_size_ = Vec2f(block_size);
			p.c_main->set_size(Vec2u(board_width, board_height));
			p.c_main->clear_cells(TileGrid);
			set_board_tiles(p.c_main);
			ui::current_entity()->add_component(p.c_main);
		ui::pop_parent();

		if (player_index == my_room_index)
		{
			block_size = 16U * scale;

			ui::next_element_pos = pos + Vec2f(22.f, 80.f);
			ui::e_text(L"Hold");

			ui::e_empty();
			ui::next_element_pos = pos + Vec2f(8.f, 100.f);
			ui::next_element_size = Vec2f(block_size * 4 + 8.f);
			{
				auto ce = ui::c_element();
				ce->inner_padding_ = Vec4f(4.f);
				ce->color_ = Vec4c(30, 30, 30, 255);
			}
			{
				p.c_hold = cTileMap::create();
				p.c_hold->cell_size_ = Vec2f(block_size);
				p.c_hold->set_size(Vec2u(4, 3));
				set_board_tiles(p.c_hold);
				ui::current_entity()->add_component(p.c_hold);
			}

			ui::next_element_pos = pos + Vec2f(350.f, 80.f);
			ui::e_text(L"Next");

			ui::e_empty();
			ui::next_element_pos = pos + Vec2f(330.f, 100.f);
			ui::next_element_size = Vec2f(block_size * 4 + 8.f, (block_size * 3.f + 4.f) * array_size(p.c_next) + 8.f - 45.f);
			{
				auto ce = ui::c_element();
				ce->color_ = Vec4c(30, 30, 30, 255);
			}
			auto create_next_board = [&](int i, int base, float y_off, float block_size) {
				ui::e_empty();
				ui::next_element_pos = pos + Vec2f(330.f, 100.f + y_off + (block_size * 3.f + 4.f) * (i - base));
				ui::next_element_size = Vec2f(block_size * 4 + 8.f);
				ui::c_element()->inner_padding_ = Vec4f(4.f);
				{
					p.c_next[i] = cTileMap::create();
					p.c_next[i]->cell_size_ = Vec2f(block_size);
					p.c_next[i]->set_size(Vec2u(4));
					set_board_tiles(p.c_next[i]);
					ui::current_entity()->add_component(p.c_next[i]);
				}
			};
			for (auto i = 0; i < 1; i++)
				create_next_board(i, 0, 0.f, 16.f);
			for (auto i = 1; i < 3; i++)
				create_next_board(i, 1, 16.f * 3.f + 4.f, 14.f);
			for (auto i = 3; i < array_size(p.c_next); i++)
				create_next_board(i, 3, 16.f * 3.f + 4.f + (14.f * 3.f + 4.f) * 2, 12.f);

			ui::next_element_pos = pos + Vec2f(180.f, 250.f);
			ui::push_style_1u(ui::FontSize, 80);
			p.c_count_down = ui::e_text(L"")->get_component(cText);
			ui::pop_style(ui::FontSize);

			if (game_mode == GameMulti)
			{
				ui::next_element_pos = pos + Vec2f(54.f, 546.f);
				p.e_garbage = ui::e_element();
			}
		}

		if (game_mode == GameMulti)
		{
			ui::push_style_1u(ui::FontSize, 60 * scale);
			ui::next_element_pos = pos + Vec2f(150.f, 200.f) * scale;
			p.c_ready = ui::e_text(L"Ready")->get_component(cText);
			p.c_ready->entity->set_visibility(false);
			ui::next_element_pos = pos + Vec2f(160.f, 150.f) * scale;
			p.c_rank = ui::e_text(L"Ready")->get_component(cText);
			p.c_rank->entity->set_visibility(false);
			ui::pop_style(ui::FontSize);
		}
	}

	void process_player_entered(int index)
	{
		looper().add_event([](void* c, bool*) {
			auto index = *(int*)c;
			auto& p = app.players[index];
			ui::push_parent(app.root);
				p.e = ui::e_element();
				ui::push_parent(p.e);
					app.create_player_controls(index);
				ui::pop_parent();
			ui::pop_parent();
		}, new_mail(&index));
	}

	void process_player_disconnected(int index)
	{
		looper().add_event([](void* c, bool*) {
			auto index = *(int*)c;
			auto& p = app.players[index];
			p.disconnected = true;
			p.c_name->set_text((p.name + L" " + Icon_BOLT).c_str());
		}, new_mail(&index));
	}

	void process_player_left(int index)
	{
		looper().add_event([](void* c, bool*) {
			auto index = *(int*)c;
			auto& p = app.players[index];
			p.reset();
			app.root ->remove_child(p.e);
		}, new_mail(&index));
	}

	void process_player_ready(int index)
	{
		looper().add_event([](void* c, bool*) {
			auto& p = app.players[*(int*)c];
			p.ready = true;
			p.c_ready->entity->set_visibility(true);
		}, new_mail(&index));
	}

	void process_game_start()
	{
		looper().add_event([](void*, bool*) {
			app.room_gaming = true;
			app.start_game();
		}, Mail<>());
	}

	void process_report_board(int index, const std::string& d)
	{
		struct Capture
		{
			cTileMap* b;
			std::string d;
		}capture;
		capture.b = app.players[index].c_main;
		capture.d = d;
		looper().add_event([](void* c, bool*) {
			auto& capture = *(Capture*)c;
			for (auto y = 0; y < board_height; y++)
			{
				for (auto x = 0; x < board_width; x++)
				{
					auto id = capture.d[y * board_width + x] - '0';
					capture.b->set_cell(Vec2u(x, y), id, id == TileGrid ? Vec4c(255) : mino_col_decay);
				}
			}
		}, new_mail(&capture));
	}

	void process_attack(int value)
	{
		looper().add_event([](void* c, bool*) {
			auto n = *(int*)c;
			for (auto i = 0; i < n; i++)
				app.garbages.push_back(60);
		}, new_mail(&value));
	}

	void process_dead(int index, int rank)
	{
		struct Capture
		{
			int index;
			int rank;
		}capture;
		capture.index = index;
		capture.rank = rank;
		looper().add_event([](void* c, bool*) {
			auto& capture = *(Capture*)c;
			auto& p = app.players[capture.index];
			std::wstring str;
			switch (capture.rank)
			{
			case 1:
				str = L"1st";
				break;
			case 2:
				str = L"2nd";
				break;
			case 3:
				str = L"3rd";
				break;
			default:
				str = std::to_wstring(capture.rank) + L"th";
			}
			p.c_rank->set_text(str.c_str());
			p.c_rank->entity->set_visibility(true);
		}, new_mail(&capture));
	}

	void process_gameover()
	{
		looper().add_event([](void*, bool*) {
			app.room_gaming = false;
			app.gaming = false;
			app.e_start_or_ready->set_visibility(true);
		}, Mail<>());
	}

	void join_room(const char* ip)
	{
		app.client = Client::create(SocketNormal, ip, 2434,
		[](void*, const char* msg, uint size) {
			auto req = nlohmann::json::parse(std::string(msg, size));
			auto action = req["action"].get<std::string>();
			if (action == "report_room")
			{
				app.room_name = s2w(req["room_name"].get<std::string>());
				app.room_max_people = req["max_people"].get<int>();
				app.players.resize(app.room_max_people);
				for (auto& p : app.players)
					p.reset();
				app.my_room_index = req["index"].get<int>();
				auto& me = app.players[app.my_room_index];
				me.id = (void*)0xffff;
				me.name = app.my_name;
				looper().add_event([](void*, bool*) {
					app.root->remove_children(1, -1);
					app.game_mode = GameMulti;
					app.create_game_scene();
				}, Mail<>());
			}
			else if (action == "player_entered")
			{
				auto index = req["index"].get<int>();
				auto& p = app.players[index];
				p.id = (void*)0xffff;
				p.name = s2w(req["name"].get<std::string>());

				app.process_player_entered(index);
			}
			else if (action == "player_disconnected")
				app.process_player_disconnected(req["index"].get<int>());
			else if (action == "player_left")
				app.process_player_left(req["index"].get<int>());
			else if (action == "player_ready")
				app.process_player_ready(req["index"].get<int>());
			else if (action == "game_start")
				app.process_game_start();
			else if (action == "report_board")
				app.process_report_board(req["index"].get<int>(), req["board"].get<std::string>());
			else if (action == "report_dead")
				app.process_dead(req["index"].get<int>(), req["rank"].get<int>());
			else if (action == "report_gameover")
				app.process_gameover();
			else if (action == "attack")
			{
				auto value = req["value"].get<int>();
				app.process_attack(value);
			}
		},
		[](void*) {
			looper().add_event([](void*, bool*) {
				ui::e_message_dialog(L"Host Has Disconnected")->on_removed_listeners.add([](void*) {
					looper().add_event([](void*, bool*) {
						app.quit_game();
					}, Mail<>());
					return true;
				}, Mail<>());
			}, Mail<>());
		}, Mail<>());
		if (app.client)
		{
			nlohmann::json req;
			req["action"] = "join_room";
			req["name"] = w2s(app.my_name);
			auto str = req.dump();
			app.client->send(str.data(), str.size());
		}
		else
			ui::e_message_dialog(L"Join Room Failed");
	}

	void people_dead(int index)
	{
		auto& p = players[index];
		p.dead = true;

		auto total_people = 0;
		auto dead_people = 0;
		auto last_people = 0;
		for (auto i = 0; i < app.players.size(); i++)
		{
			auto& p = app.players[i];
			if (p.id)
			{
				total_people++;
				if (p.dead)
					dead_people++;
				else
					last_people = i;
			}
		}
		auto rank = total_people - dead_people + 1;

		{
			nlohmann::json rep;
			rep["action"] = "report_dead";
			rep["index"] = index;
			rep["rank"] = rank;
			auto str = rep.dump();
			for (auto i = 1; i < app.players.size(); i++)
			{
				auto& p = app.players[i];
				if (p.id && !p.disconnected)
					app.server->send(p.id, str.data(), str.size(), false);
			}
		}

		process_dead(index, rank);

		if (total_people - dead_people == 1)
		{
			app.process_dead(last_people, 1);

			{
				nlohmann::json rep;
				rep["action"] = "report_dead";
				rep["index"] = last_people;
				rep["rank"] = 1;
				auto str = rep.dump();
				for (auto i = 1; i < app.players.size(); i++)
				{
					auto& p = app.players[i];
					if (p.id && !p.disconnected)
						app.server->send(p.id, str.data(), str.size(), false);
				}
			}

			for (auto i = 1; i < app.players.size(); i++)
			{
				auto& p = app.players[i];
				if (p.id)
					p.e_kick->set_visibility(true);
			}
			app.process_gameover();

			{
				nlohmann::json rep;
				rep["action"] = "report_gameover";
				auto str = rep.dump();
				for (auto i = 1; i < app.players.size(); i++)
				{
					auto& p = app.players[i];
					if (p.id && !p.disconnected)
						app.server->send(p.id, str.data(), str.size(), false);
				}
			}
		}
	}

	void create_lan_scene()
	{
		ui::push_parent(root);
		ui::next_element_size = Vec2f(500.f, 0.f);
		ui::e_begin_layout(LayoutVertical, 8.f, false, false)->get_component(cElement)->inner_padding_ = 8.f;
		ui::c_aligner(SizeFixed, SizeFitParent)->x_align_ = AlignxMiddle;
			ui::push_style_1u(ui::FontSize, 20);
			ui::e_begin_layout(LayoutHorizontal, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_text(L"Your Name");
				ui::e_edit(300.f, app.my_name.c_str())->get_component(cText)->data_changed_listeners.add([](void*, Component* c, uint hash, void*) {
					if (hash == FLAME_CHASH("text"))
						app.my_name = ((cText*)c)->text();
					return true;
				}, Mail<>());
			ui::e_end_layout();
			ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(0.f), 4.f, 2.f);
				auto e_room_list = ui::e_begin_list(true);
				ui::e_end_list();
			ui::e_end_scroll_view1();
			ui::e_begin_layout(LayoutHorizontal, 8.f)->get_component(cLayout)->fence = 4;
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
							auto rep = nlohmann::json::parse(std::string(msg, size));
							auto name = s2w(rep["name"].get<std::string>());
							auto host = s2w(rep["host"].get<std::string>());

							ui::push_parent(e_room_list);
							ui::e_list_item((L"Name:" + name + L" Host:" + host).c_str());
							auto dk = cDataKeeper::create();
							dk->add_stringa_item(FLAME_CHASH("ip"), ip);
							ui::current_entity()->add_component(dk);
							ui::pop_parent();
						}, new_mail_p(e_room_list));
					}, new_mail_p(e_room_list));
				}, new_mail_p(e_room_list))->get_component(cEventReceiver)->on_mouse(KeyStateDown | KeyStateUp, Mouse_Null, Vec2i(0));
				ui::e_button(L"Create Room", [](void*) {
					if (app.my_name.empty())
						ui::e_message_dialog(L"Your Name Cannot Not Be Empty");
					else
					{
						ui::e_begin_dialog();
							ui::e_text(L"Room Name");
							ui::e_edit(100.f)->get_component(cText)->data_changed_listeners.add([](void*, Component* c, uint hash, void*) {
								if (hash == FLAME_CHASH("text"))
									app.room_name = ((cText*)c)->text();
								return true;
							}, Mail<>());
							ui::e_text(L"Max People");
							ui::e_begin_combobox(100.f)->get_component(cCombobox)->data_changed_listeners.add([](void*, Component* c, uint hash, void*) {
								if (hash == FLAME_CHASH("index"))
								{
									auto index = ((cCombobox*)c)->idx;
									switch (index)
									{
									case 0:
										app.room_max_people = 2;
										break;
									case 1:
										app.room_max_people = 7;
										break;
									}
								}
								return true;
							}, Mail<>());
							ui::e_combobox_item(L"2");
							ui::e_combobox_item(L"7");
							ui::e_end_combobox(0);
							app.room_max_people = 2;
							ui::e_begin_layout(LayoutHorizontal, 4.f);
							ui::c_aligner(AlignxMiddle, AlignyFree);
								ui::e_button(L"OK", [](void* c) {
									ui::remove_top_layer(app.root);

									if (!app.room_name.empty())
									{
										app.players.resize(app.room_max_people);
										for (auto& p : app.players)
											p.reset();
										app.my_room_index = 0;
										{
											auto& me = app.players[0];
											me.id = (void*)0xffff;
											me.name = app.my_name;
										}
										app.server = Server::create(SocketNormal, 2434,
										[](void*, void* id, const char* msg, uint size) {
											auto req = nlohmann::json::parse(std::string(msg, size));
											if (req["action"] == "get_room")
											{
												nlohmann::json rep;
												rep["name"] = w2s(app.room_name);
												rep["host"] = w2s(app.my_name);
												auto str = rep.dump();
												app.server->send(id, str.data(), str.size(), true);
											}
										},
										[](void*, void* id) {
											if (!app.room_gaming)
											{
												for (auto i = 0; i < app.players.size(); i++)
												{
													auto& p = app.players[i];
													if (!p.id)
													{
														{
															nlohmann::json rep;
															rep["action"] = "report_room";
															rep["room_name"] = w2s(app.room_name);
															rep["max_people"] = app.room_max_people;
															rep["index"] = i;
															auto str = rep.dump();
															app.server->send(id, str.data(), str.size(), false);
														}
														{
															nlohmann::json rep;
															rep["action"] = "player_entered";
															rep["index"] = app.my_room_index;
															rep["name"] = w2s(app.my_name);
															auto str = rep.dump();
															app.server->send(id, str.data(), str.size(), false);
														}

														p.id = id;
														app.server->set_client(id,
														[](void* c, const char* msg, uint size) {
															auto index = *(int*)c;
															auto& p = app.players[index];
															auto req = nlohmann::json::parse(std::string(msg, size));
															auto action = req["action"].get<std::string>();
															if (action == "join_room")
															{
																p.name = s2w(req["name"].get<std::string>());

																app.process_player_entered(index);

																{
																	nlohmann::json rep;
																	rep["action"] = "player_entered";
																	rep["index"] = index;
																	rep["name"] = w2s(p.name);
																	auto str = rep.dump();
																	for (auto i = 1; i < app.players.size(); i++)
																	{
																		if (i != index)
																		{
																			auto& p = app.players[i];
																			if (p.id && !p.disconnected)
																				app.server->send(p.id, str.data(), str.size(), false);
																		}
																	}
																}
															}
															else if (action == "ready")
															{
																app.process_player_ready(index);

																{
																	nlohmann::json rep;
																	rep["action"] = "player_ready";
																	rep["index"] = index;
																	auto str = rep.dump();
																	for (auto i = 1; i < app.players.size(); i++)
																	{
																		if (i != index)
																		{
																			auto& p = app.players[i];
																			if (p.id && !p.disconnected)
																				app.server->send(p.id, str.data(), str.size(), false);
																		}
																	}
																}
															}
															else if (action == "report_board")
															{
																auto d = req["board"].get<std::string>();
																app.process_report_board(req["index"].get<int>(), d);

																{
																	nlohmann::json rep;
																	rep["action"] = "report_board";
																	rep["index"] = index;
																	rep["board"] = d;
																	auto str = rep.dump();
																	for (auto i = 1; i < app.players.size(); i++)
																	{
																		if (i != index)
																		{
																			auto& p = app.players[i];
																			if (p.id && !p.disconnected)
																				app.server->send(p.id, str.data(), str.size(), false);
																		}
																	}
																}
															}
															else if (action == "report_dead")
																app.people_dead(index);
															else if (action == "attack")
															{
																auto value = req["value"].get<int>();
																app.process_attack(value);
															}
														},
														[](void* c) {
															auto index = *(int*)c;

															app.process_player_disconnected(index);

															{
																nlohmann::json rep;
																rep["action"] = "player_disconnected";
																rep["index"] = index;
																auto str = rep.dump();
																for (auto i = 1; i < app.players.size(); i++)
																{
																	if (i != index)
																	{
																		auto& p = app.players[i];
																		if (p.id && !p.disconnected)
																			app.server->send(p.id, str.data(), str.size(), false);
																	}
																}
															}

															if (app.room_gaming)
																app.people_dead(index);
														}, new_mail(&i));

														break;
													}
												}
											}
										}, Mail<>());
										app.room_gaming = false;
										looper().add_event([](void*, bool*) {
											app.root->remove_children(1, -1);
											app.game_mode = GameMulti;
											app.create_game_scene();
										}, Mail<>());
									}
								}, Mail<>());
								ui::e_button(L"Cancel", [](void* c) {
									ui::remove_top_layer(app.root);
								}, Mail<>());
							ui::e_end_layout();
						ui::e_end_dialog();
					}
				}, Mail<>());
				ui::e_button(L"Join Room", [](void* c) {
					auto e_room_list = *(Entity**)c;
					auto selected = e_room_list->get_component(cList)->selected;
					if (selected)
					{
						if (app.my_name.empty())
							ui::e_message_dialog(L"Your Name Cannot Not Be Empty");
						else
							app.join_room(selected->get_component(cDataKeeper)->get_stringa_item(FLAME_CHASH("ip")));
					}
					else
						ui::e_message_dialog(L"You Need To Select A Room");
				}, new_mail_p(e_room_list));
				ui::e_button(L"Direct Connect", [](void* c) {
					if (app.my_name.empty())
						ui::e_message_dialog(L"Your Name Cannot Not Be Empty");
					else
					{
						ui::e_input_dialog(L"IP", [](void*, bool ok, const wchar_t* text) {
							if (ok)
								app.join_room(w2s(text).c_str());
						}, Mail<>());
					}
				}, new_mail_p(e_room_list));
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_home_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxRight, AlignyTop);
			ui::e_end_layout();
			ui::pop_style(ui::FontSize);
		ui::e_end_layout();
		ui::pop_parent();
	}

	void create_config_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				ui::e_button(L"Key", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_key_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Sound", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_sound_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::e_button(L"Sensitiveness", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_sensitiveness_scene();
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

	void create_key_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				auto key_info = find_enum(FLAME_CHASH("flame::Key"));
				for (auto i = 0; i < KEY_COUNT; i++)
				{
					ui::e_begin_layout(LayoutHorizontal, 4.f);
					ui::e_text(key_names[i]);
					struct Capture
					{
						cText* t;
						int i;
					}capture;
					auto e_edit = ui::e_edit(200.f, s2w(key_info->find_item(key_map[i])->name()).c_str());
					capture.t = e_edit->get_component(cText);
					capture.i = i;
					e_edit->get_component(cEventReceiver)->key_listeners.add([](void* c, KeyStateFlags action, int value) {
						if (action == KeyStateDown)
						{
							auto& capture = *(Capture*)c;
							key_map[capture.i] = (Key)value;
							auto key_info = find_enum(FLAME_CHASH("flame::Key"));
							capture.t->set_text(s2w(key_info->find_item((Key)value)->name()).c_str());
						}
						return false;
					}, new_mail(&capture), 0);
					ui::c_aligner(SizeGreedy, SizeFixed);
					ui::e_end_layout();
				}
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_config_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_sound_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				struct Capture
				{
					cText* t;
					int v;
				}capture;
				ui::e_begin_layout(LayoutHorizontal, 4.f);
					capture.t = ui::e_text(wfmt(L"FX %d", fx_volumn).c_str())->get_component(cText);
					auto change_fx_volumn = [](void* c) {
						auto& capture = *(Capture*)c;
						auto v = fx_volumn + capture.v;
						if (v >= 0 && v <= 10)
						{
							fx_volumn = v;
							capture.t->set_text(wfmt(L"FX %d", v).c_str());

							auto f = v / 10.f;
							app.sound_move_src->set_volume(f * sound_move_volumn);
							app.sound_soft_drop_src->set_volume(f* sound_soft_drop_volumn);
							app.sound_hard_drop_src->set_volume(f* sound_hard_drop_volumn);
							app.sound_clear_src->set_volume(f* sound_clear_volumn);
							app.sound_hold_src->set_volume(f* sound_hold_volumn);
						}
					};
					capture.v = -1;
					ui::e_button(L"-", change_fx_volumn, new_mail(&capture));
					capture.v = 1;
					ui::e_button(L"+", change_fx_volumn, new_mail(&capture));
				ui::e_end_layout();
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_config_scene();
					}, Mail<>());
				}, Mail<>());
				ui::c_aligner(AlignxMiddle, AlignyFree);
				ui::pop_style(ui::FontSize);
			ui::e_end_layout();
		ui::pop_parent();
	}

	void create_sensitiveness_scene()
	{
		ui::push_parent(root);
			ui::e_begin_layout(LayoutVertical, 8.f);
			ui::c_aligner(AlignxMiddle, AlignyMiddle);
				ui::push_style_1u(ui::FontSize, 20);
				ui::e_text(L"Small Number Means More Sensitivity Or Faster");
				struct Capture
				{
					cText* t;
					int v;
				}capture;
				ui::e_begin_layout(LayoutHorizontal, 4.f);
					capture.t = ui::e_text(wfmt(L"Left Right Sensitiveness %d",
						left_right_sensitiveness).c_str())->get_component(cText);
					auto change_lr_sens = [](void* c) {
						auto& capture = *(Capture*)c;
						auto v = left_right_sensitiveness + capture.v;
						if (v >= 5 && v <= 30)
						{
							left_right_sensitiveness = v;
							capture.t->set_text(wfmt(L"Left Right Sensitiveness %d", v).c_str());
						}
					};
					capture.v = -1;
					ui::e_button(L"-", change_lr_sens, new_mail(&capture));
					capture.v = 1;
					ui::e_button(L"+", change_lr_sens, new_mail(&capture));
				ui::e_end_layout();
				ui::e_begin_layout(LayoutHorizontal, 4.f);
					capture.t = ui::e_text(wfmt(L"Left Right Speed %d",
						left_right_speed).c_str())->get_component(cText);
					auto change_lr_sp = [](void* c) {
						auto& capture = *(Capture*)c;
						auto v = left_right_speed + capture.v;
						if (v >= 1 && v <= 10)
						{
							left_right_speed = v;
							capture.t->set_text(wfmt(L"Left Right Speed %d", v).c_str());
						}
					};
					capture.v = -1;
					ui::e_button(L"-", change_lr_sp, new_mail(&capture));
					capture.v = 1;
					ui::e_button(L"+", change_lr_sp, new_mail(&capture));
				ui::e_end_layout();
				ui::e_begin_layout(LayoutHorizontal, 4.f);
					capture.t = ui::e_text(wfmt(L"Soft Drop Speed %d",
						soft_drop_speed).c_str())->get_component(cText);
					auto change_sd_sp = [](void* c) {
						auto& capture = *(Capture*)c;
						auto v = soft_drop_speed + capture.v;
						if (v >= 1 && v <= 10)
						{
							soft_drop_speed = v;
							capture.t->set_text(wfmt(L"Soft Drop Speed %d", v).c_str());
						}
					};
					capture.v = -1;
					ui::e_button(L"-", change_sd_sp, new_mail(&capture));
					capture.v = 1;
					ui::e_button(L"+", change_sd_sp, new_mail(&capture));
				ui::e_end_layout();
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.root->remove_children(1, -1);
						app.create_config_scene();
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

			{
				auto e = ui::e_empty();
				e->on_destroyed_listeners.add([](void* c) {
					looper().remove_event(*(void**)c);
					return true;
				}, new_mail_p(looper().add_event([](void*, bool* go_on) {
					app.do_game_logic();
					*go_on = true;
				}, Mail<>())));
			}

			if (game_mode == GameMulti)
				ui::e_text(wfmt(L"Room: %s", room_name.c_str()).c_str());

				create_player_controls(my_room_index);

				if (game_mode != GameMulti)
				{
					ui::next_element_pos = Vec2f(535.f, 150.f);
					ui::e_text(L"TIME")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(535.f, 210.f);
					ui::e_text(L"LEVEL")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(535.f, 270.f);
					ui::e_text(game_mode == GameSingleRTA ? L"LEFT" : L"LINES")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(535.f, 330.f);
					ui::e_text(L"SCORE")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::push_style_1u(ui::FontSize, 40);
					ui::next_element_pos = Vec2f(535.f, 170.f);
					c_text_time = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(535.f, 230.f);
					c_text_level = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(535.f, 290.f);
					c_text_lines = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(535.f, 350.f);
					c_text_score = ui::e_text(L"")->get_component(cText);
					ui::pop_style(ui::FontSize);
				}

				ui::push_style_1u(ui::FontSize, 28);
				ui::next_element_pos = Vec2f(8.f, 230.f);
				{
					auto e = ui::e_text(L"");
					e->set_visibility(false);
					c_text_special = e->get_component(cText);
				}
				c_text_special->color = Vec4c(200, 80, 40, 255);
				ui::pop_style(ui::FontSize);

			if (game_mode == GameMulti)
			{
				ui::next_element_pos = Vec2f(4.f, 500.f);
				if (my_room_index == 0)
				{
					e_start_or_ready = ui::e_button(L"Start", [](void*) {
						if ([]() {
							auto only_you = true;
							for (auto i = 1; i < app.players.size(); i++)
							{
								auto& p = app.players[i];
								if (!p.id)
									continue;
								only_you = false;
								if (!p.ready)
									return false;
							}
							return !only_you;
						}())
						{
							for (auto i = 1; i < app.players.size(); i++)
							{
								auto& p = app.players[i];
								if (p.id && !p.disconnected)
									p.e_kick->set_visibility(false);
							}
							app.process_game_start();

							{
								nlohmann::json req;
								req["action"] = "game_start";
								auto str = req.dump();
								for (auto i = 1; i < app.players.size(); i++)
								{
									auto& p = app.players[i];
									if (p.id && !p.disconnected)
										app.server->send(p.id, str.data(), str.size(), false);
								}
							}
						}
					}, Mail<>());
				}
				else
				{
					e_start_or_ready = ui::e_button(L"Ready", [](void*) {
						auto& me = app.players[app.my_room_index];
						if (!app.room_gaming && !me.ready)
						{
							me.ready = true;
							nlohmann::json req;
							req["action"] = "ready";
							auto str = req.dump();
							app.client->send(str.data(), str.size());

							app.process_player_ready(app.my_room_index);
						}
					}, Mail<>());
				}
			}

			ui::e_button(Icon_TIMES, [](void*) {
				ui::e_confirm_dialog(L"Quit?", [](void*, bool yes) {
					if (yes)
					{
						looper().add_event([](void*, bool*) {
							app.quit_game();
						}, Mail<>());
					}
				}, Mail<>());
			}, Mail<>());
			ui::c_aligner(AlignxRight, AlignyTop);

		ui::pop_style(ui::FontSize);
		ui::pop_parent();
	}

	void begin_count_down()
	{
		struct Capture
		{
			uint time;
			cText* text;

			~Capture()
			{
				text->entity->set_visibility(false);
			}
		};
		auto capture = new_mail<Capture>();
		capture.p->time = 3;
		capture.p->text = players[my_room_index].c_count_down;
		capture.p->text->set_text(L"3");
		capture.p->text->entity->set_visibility(true);
		looper().add_event([](void* c, bool* go_on) {
			auto& capture = *(Capture*)c;
			capture.time--;
			capture.text->set_text(std::to_wstring(capture.time).c_str());
			if (capture.time > 0)
				*go_on = true;
			else
				app.gaming = true;
		}, capture, 1.f, FLAME_CHASH("count_down"));
	}

	void update_status()
	{
		if (game_mode != GameMulti)
		{
			c_text_time->set_text(wfmt(L"%02d:%02d.%02d", (int)play_time / 60, ((int)play_time) % 60, int(play_time * 100) % 100).c_str());
			c_text_level->set_text(wfmt(L"%02d", level).c_str());
			if (game_mode == GameSingleRTA)
				c_text_lines->set_text(wfmt(L"%02d", max(0, 40 - (int)lines)).c_str());
			else
				c_text_lines->set_text(wfmt(L"%04d", lines).c_str());
			c_text_score->set_text(wfmt(L"%09d", score).c_str());
		}
	}

	void start_game()
	{
		for (auto i = 0; i < players.size(); i++)
		{
			auto& p = app.players[i];
			if (p.id)
			{
				p.ready = false;
				p.dead = false;
				p.c_main->clear_cells(TileGrid);
				if (i == app.my_room_index)
				{
					p.c_hold->clear_cells(-1);
					for (auto j = 0; j < array_size(p.c_next); j++)
						p.c_next[j]->clear_cells(-1);
				}
				if (game_mode == GameMulti)
				{
					p.c_ready->entity->set_visibility(false);
					p.c_rank->entity->set_visibility(false);
				}
			}
		}
		if (game_mode == GameMulti)
			e_start_or_ready->set_visibility(false);

		left_frames = -1;
		right_frames = -1;

		play_time = 0.f;
		level = 1;
		lines = 0;
		score = 0;
		clear_ticks = -1;
		combo = 0;
		back_to_back = false;
		garbages.clear();
		mino_pos = Vec2i(0, -1);
		mino_type = MinoTypeCount;
		mino_hold = MinoTypeCount;
		mino_just_hold = false;
		mino_pack_idx = Vec2u(0, 0);
		mino_rotation = 0;
		mino_reset_times = -1;
		mino_bottom_dist = 0;
		mino_ticks = 0;
		{
			auto seed = ::time(0);
			if (server)
				seed++;
			else if (client)
				seed--;
			srand(seed);
		}
		for (auto i = 0; i < 2; i++)
			shuffle_pack(i);

		update_status();

		gaming = false;
		begin_count_down();
	}

	void shuffle_pack(uint idx)
	{
		auto& curr_pack = mino_packs[idx];
		for (auto i = 0; i < MinoTypeCount; i++)
			curr_pack[i] = (MinoType)i;
		for (auto i = 0; i < MinoTypeCount; i++)
			std::swap(curr_pack[i], curr_pack[rand() % MinoTypeCount]);
	}

	void draw_mino(cTileMap* board, int idx, const Vec2i& pos, uint offset_y, Vec2i* coords, const Vec4c& col = Vec4c(255))
	{
		board->set_cell(Vec2u(pos) + Vec2u(0, offset_y), idx, col);
		board->set_cell(Vec2u(pos + coords[0] + Vec2u(0, offset_y)), idx, col);
		board->set_cell(Vec2u(pos + coords[1] + Vec2u(0, offset_y)), idx, col);
		board->set_cell(Vec2u(pos + coords[2] + Vec2u(0, offset_y)), idx, col);
	}

	bool check_board(cTileMap* board, const Vec2i& p)
	{
		return 
			board->cell(mino_pos + p) == TileGrid &&
			board->cell(mino_pos + p + mino_coords[0]) == TileGrid &&
			board->cell(mino_pos + p + mino_coords[1]) == TileGrid &&
			board->cell(mino_pos + p + mino_coords[2]) == TileGrid;
	}

	bool check_board(cTileMap* board, Vec2i* in, const Vec2i& p)
	{
		return
			board->cell(p) == TileGrid &&
			board->cell(in[0] + p) == TileGrid &&
			board->cell(in[1] + p) == TileGrid &&
			board->cell(in[2] + p) == TileGrid;
	}

	bool line_empty(cTileMap* board, uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2i(x, l)) != TileGrid)
				return false;
		}
		return true;
	}

	bool line_full(cTileMap* board, uint l)
	{
		for (auto x = 0; x < board_width; x++)
		{
			if (board->cell(Vec2i(x, l)) == TileGrid)
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

	bool super_rotation(cTileMap* board, bool clockwise, Vec2i* out_coord, Vec2i* offset)
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
			if (check_board(board, out_coord, mino_pos + *offset))
				return true;
		}
		return false;
	}

	void quit_game()
	{
		looper().clear_events(FLAME_CHASH("count_down"));

		room_gaming = false;
		gaming = false;

		players.resize(1);
		players[0].id = (void*)0xffff;
		my_room_index = 0;
		if (server)
		{
			Server::destroy(server);
			server = nullptr;
		}
		if (client)
		{
			Client::destroy(client);
			client = nullptr;
		}

		looper().add_event([](void*, bool*) {
			app.root->remove_children(1, -1);
			app.create_home_scene();
		}, Mail<>());
	}

	void do_game_logic()
	{
		auto& key_states = s_event_dispatcher->key_states;

		if (game_mode != GameMulti)
		{
			if (key_states[key_map[KEY_PAUSE]] == (KeyStateDown | KeyStateJust))
			{
				if (!ui::get_top_layer(app.root, true, "paused"))
				{
					looper().clear_events(FLAME_CHASH("count_down"));
					gaming = false;

					ui::e_begin_dialog("paused");
					ui::e_text(L"Paused");
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Resume", [](void*) {
						ui::remove_top_layer(app.root);
						app.begin_count_down();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Restart", [](void*) {
						ui::remove_top_layer(app.root);
						app.play_time = 0.f;
						app.start_game();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Quit", [](void*) {
						ui::remove_top_layer(app.root);
						app.quit_game();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_end_dialog();
				}
				else
				{
					ui::remove_top_layer(app.root);
					begin_count_down();
				}
			}
		}

		if (gaming)
		{
			play_time += looper().delta_time;

			auto& p = players[my_room_index];
			auto& c_main = p.c_main;
			auto& c_hold = p.c_hold;
			auto& c_next = p.c_next;
			auto& e_garbage = p.e_garbage;

			auto dead = false;

			if (clear_ticks != -1)
			{
				clear_ticks--;
				if (clear_ticks <= 0)
				{
					for (auto i = 0; i < 4; i++)
					{
						auto l = full_lines[i];
						if (l != -1)
						{
							for (auto j = (int)l; j >= 0; j--)
							{
								if (j > 0)
								{
									for (auto x = 0; x < board_width; x++)
									{
										auto id = c_main->cell(Vec2i(x, j - 1));
										c_main->set_cell(Vec2u(x, j), id, id == TileGrid ? Vec4c(255) : mino_col_decay);
									}
								}
								else
								{
									for (auto x = 0; x < board_width; x++)
										c_main->set_cell(Vec2u(x, j), TileGrid);
								}
							}
						}
					}

					clear_ticks = -1;
				}
			}
			else
			{
				if (mino_pos.y() >= 0)
				{
					draw_mino(c_main, TileGrid, mino_pos, 0, mino_coords);
					if (mino_bottom_dist > 0)
						draw_mino(c_main, TileGrid, mino_pos, mino_bottom_dist, mino_coords);
				}

				if (mino_pos.y() < 0)
				{
					if (mino_pos.y() == -1 || mino_type == MinoTypeCount)
					{
						mino_type = mino_packs[mino_pack_idx.x()][mino_pack_idx.y()++];
						if (mino_pack_idx.y() >= MinoTypeCount)
						{
							shuffle_pack(mino_pack_idx.x());
							mino_pack_idx = Vec2i(1 - mino_pack_idx.x(), 0);
						}
						for (auto i = 0; i < array_size(c_next); i++)
						{
							c_next[i]->clear_cells();
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
							draw_mino(c_next[i], TileMino1 + t, Vec2i(1), 0, coords);
						}
					}
					if (mino_pos.y() == -2)
					{
						c_hold->clear_cells();
						if (mino_hold != MinoTypeCount)
						{
							Vec2i coords[3];
							for (auto i = 0; i < 3; i++)
								coords[i] = g_mino_coords[mino_hold][i];
							draw_mino(c_hold, TileMino1 + mino_hold, Vec2i(1), 0, coords);
						}
					}
					mino_pos = Vec2i(4, 5 - (mino_type == Mino_I ? 1 : 0));
					mino_rotation = 0;
					for (auto i = 0; i < 3; i++)
						mino_coords[i] = g_mino_coords[mino_type][i];
					mino_reset_times = -1;
					mino_ticks = 0;

					dead = !check_board(c_main, Vec2i(0));
					if (dead)
					{
						{
							auto pos = mino_pos;
							c_main->set_cell(Vec2u(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[0];
							c_main->set_cell(Vec2u(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[1];
							c_main->set_cell(Vec2u(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[2];
							c_main->set_cell(Vec2u(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
					}
					if (dead || (game_mode == GameSingleRTA && lines >= 40))
					{
						gaming = false;

						if (game_mode != GameMulti)
						{
							ui::e_begin_dialog();
							ui::e_text(L"Game Over");
							ui::c_aligner(AlignxMiddle, AlignyFree);
							ui::e_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
							ui::e_text((L"Level: " + wfmt(L"%d", level)).c_str());
							ui::e_text((L"Lines: " + wfmt(L"%d", lines)).c_str());
							ui::e_text((L"Score: " + wfmt(L"%d", score)).c_str());
							ui::e_button(L"Quit", [](void*) {
								ui::remove_top_layer(app.root);
								app.quit_game();
							}, Mail<>());
							ui::c_aligner(AlignxMiddle, AlignyFree);
							ui::e_button(L"Restart", [](void*) {
								ui::remove_top_layer(app.root);
								app.start_game();
							}, Mail<>());
							ui::c_aligner(AlignxMiddle, AlignyFree);
							ui::e_end_dialog();
						}
					}
				}

				if (!dead)
				{
					if (key_states[key_map[KEY_HOLD]] == (KeyStateDown | KeyStateJust) && (game_mode == GameSinglePractice || mino_just_hold == false))
					{
						mino_pos.y() = -2;
						std::swap(mino_hold, mino_type);
						mino_just_hold = true;

						sound_hold_src->play();
					}
					else
					{
						static auto last_is_rotate_action = false;
						static auto mini = false;

						auto rotated = false;

						auto r = 0;
						if (key_states[key_map[KEY_ROTATE_LEFT]] == (KeyStateDown | KeyStateJust))
							r--;
						if (key_states[key_map[KEY_ROTATE_RIGHT]] == (KeyStateDown | KeyStateJust))
							r++;
						if (r != 0)
						{
							Vec2i new_coords[3];
							Vec2i offset;
							if (super_rotation(c_main, r == 1, new_coords, &offset))
							{
								if (offset != 0)
									mini = true;
								else
									mini = false;
								mino_rotation = get_rotation_idx(r == 1);
								mino_pos += offset;
								for (auto i = 0; i < 3; i++)
									mino_coords[i] = new_coords[i];
								rotated = true;

								sound_move_src->play();
							}
						}

						auto moved = false;

						auto mx = 0;
						if (key_states[key_map[KEY_LEFT]] & KeyStateDown)
						{
							if (left_frames == -1)
								left_frames = 0;
							else
								left_frames++;
							if (left_frames == 0 || (left_frames >= left_right_sensitiveness
								&& left_frames % left_right_speed == 0))
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
							if (right_frames == 0 || (right_frames >= left_right_sensitiveness
								&& right_frames % left_right_speed == 0))
								mx++;
						}
						else
							right_frames = -1;
						if (mx != 0 && check_board(c_main, Vec2i(mx, 0)))
						{
							mino_pos.x() += mx;
							moved = true;

							sound_move_src->play();
						}

						if (!last_is_rotate_action)
							last_is_rotate_action = rotated && !moved;
						else
							last_is_rotate_action = !moved;

						mino_bottom_dist = 0;
						while (check_board(c_main, Vec2i(0, mino_bottom_dist + 1)))
							mino_bottom_dist++;
						if (rotated || moved)
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
										mino_ticks = DOWN_TICKS;
									else
										mino_ticks = 0;
									mino_reset_times++;
								}
							}
						}
						auto is_soft_drop = key_states[key_map[KEY_SOFT_DROP]] & KeyStateDown;
						auto down_ticks_final = DOWN_TICKS;
						if (game_mode == GameSinglePractice)
							down_ticks_final = 9999;
						else
							down_ticks_final = down_ticks_final - level + 1;
						if (mino_bottom_dist == 0)
							down_ticks_final = 30;
						else if (is_soft_drop)
							down_ticks_final = soft_drop_speed;
						auto hard_drop = key_states[key_map[KEY_HARD_DROP]] == (KeyStateDown | KeyStateJust);
						if (hard_drop || mino_ticks >= down_ticks_final)
						{
							if (mino_bottom_dist > 0)
								last_is_rotate_action = false;

							if (hard_drop || mino_bottom_dist == 0)
							{
								mino_pos.y() += mino_bottom_dist;
								if (hard_drop)
									score += mino_bottom_dist * 2;
								mino_bottom_dist = 0;
								draw_mino(c_main, TileMino1 + mino_type, mino_pos, 0, mino_coords, mino_col_decay);

								for (auto i = 0; i < 4; i++)
									full_lines[i] = -1;
								auto l = 0U;
								for (auto i = 0; i < board_height; i++)
								{
									if (line_full(c_main, i))
									{
										full_lines[l] = i;

										{
											auto cell_size = c_main->cell_size_;
											auto board_element = c_main->element;
											auto pos = board_element->global_pos + Vec2f(board_element->inner_padding_[0], board_element->inner_padding_[1]);
											pos.y() += i * cell_size.y();
											ui::push_parent(root);
											ui::next_element_pos = pos;
											ui::next_element_size = Vec2f(cell_size.x() * board_width, cell_size.y());
											ui::e_empty();
											auto element = ui::c_element();
											element->color_ = Vec4c(255);
											ui::pop_parent();

											struct Capture
											{
												cElement* e;
												uint f;
											}capture;
											capture.e = element;
											capture.f = 5;
											looper().add_event([](void* c, bool* go_on) {
												auto& capture = *(Capture*)c;
												capture.f--;
												if (capture.f > 0)
												{
													capture.e->pos_.x() -= 10.f;
													capture.e->size_.x() += 20.f;
													capture.e->pos_.y() += 2.4f;
													capture.e->size_.y() -= 4.8f;
													capture.e->color_.a() = max(capture.e->color_.a() - 30, 0);
													*go_on = true;
												}
												else
												{
													auto e = capture.e->entity;
													e->parent()->remove_child(e);
												}
											}, new_mail(&capture), 0.f);
										}

										l++;
									}
								}
								lines += l;
								if (l > 0)
								{
									std::wstring special_str;

									auto attack = 0;

									get_combo_award(combo, attack, special_str);

									combo++;

									auto tspin = mino_type == Mino_T && last_is_rotate_action;
									if (tspin)
									{
										Vec2i judge_points[] = {
											Vec2i(-1, -1),
											Vec2i(+1, -1),
											Vec2i(-1, +1),
											Vec2i(+1, +1),
										};
										auto count = 0;
										for (auto i = 0; i < array_size(judge_points); i++)
										{
											auto p = mino_pos + judge_points[i];
											if (p.x() < 0 || p.x() >= board_width ||
												p.y() < 0 || p.y() >= board_height ||
												c_main->cell(p) != TileGrid)
												count++;
										}
										if (count < 3)
											tspin = false;
									}

									get_lines_award(l, tspin, mini, back_to_back, score, attack, special_str);

									auto cancel = max(attack, (int)l);
									if (!garbages.empty())
									{
										if (garbages.size() < cancel)
										{
											cancel -= garbages.size();
											garbages.clear();
										}
										else
										{
											for (auto i = 0; i < cancel; i++)
												garbages.erase(garbages.begin());
											cancel = 0;
										}

										attack -= cancel;
									}

									if (!special_str.empty())
									{
										c_text_special->entity->set_visibility(true);
										c_text_special->set_text(special_str.c_str());
										looper().clear_events(FLAME_CHASH("special_text"));
										looper().add_event([](void*, bool*) {
											app.c_text_special->entity->set_visibility(false);
										}, Mail<>(), 1.f, FLAME_CHASH("special_text"));
									}

									if (attack > 0)
									{
										nlohmann::json req;
										req["action"] = "attack";
										req["value"] = attack;
										auto str = req.dump();
										if (server) 
										{
											for (auto i = 1; i < players.size(); i++)
											{
												auto& p = players[i];
												if (p.id && !p.disconnected)
													server->send(p.id, str.data(), str.size(), false);
											}
										}
										if (client)
											client->send(str.data(), str.size());
									}

									for (auto i = 0; i < l; i++)
									{
										for (auto x = 0; x < board_width; x++)
											c_main->set_cell(Vec2u(x, full_lines[i]), TileGrid);
									}

									clear_ticks = CLEAR_TICKS;
									if (game_mode == GameSingleMarathon && lines % 5 == 0)
									{
										level++;
										level = min(DOWN_TICKS, level);
									}

									sound_clear_src->play();
								}
								else
								{
									if (!garbages.empty())
									{
										auto n = 0;
										for (auto it = garbages.begin(); it != garbages.end();)
										{
											if (*it == 0)
											{
												it = garbages.erase(it);
												n++;
											}
											else
												it++;
										}

										for (auto i = 0; i < board_height - n; i++)
										{
											for (auto x = 0; x < board_width; x++)
											{
												auto id = c_main->cell(Vec2i(x, i + n));
												c_main->set_cell(Vec2u(x, i), id, id == TileGrid ? Vec4c(255) : mino_col_decay);
											}
										}
										auto hole = ::rand() % board_width;
										for (auto i = 0; i < n; i++)
										{
											auto y = board_height - i - 1;
											for (auto x = 0; x < board_width; x++)
												c_main->set_cell(Vec2u(x, y), TileGray, mino_col_decay);
											c_main->set_cell(Vec2u(hole, y), TileGrid);
										}
									}

									clear_ticks = 0;
									combo = 0;

									if (hard_drop)
										sound_hard_drop_src->play();
									else
										sound_soft_drop_src->play();
								}
								mino_pos.y() = -1;
								mino_just_hold = false;
							}
							else
							{
								mino_pos.y()++;
								mino_bottom_dist--;
								if (is_soft_drop)
								{
									score++;

									sound_move_src->play();
								}
							}
							mino_ticks = 0;
						}
						mino_ticks++;

						if (mino_pos.y() != -1)
						{
							if (mino_bottom_dist)
								draw_mino(c_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords, g_mino_colors[mino_type]);
							draw_mino(c_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
						}
					}
				}

				if (game_mode == GameMulti)
				{
					nlohmann::json req;
					req["action"] = "report_board";
					req["index"] = my_room_index;
					std::string d;
					d.resize(board_height * board_width);
					for (auto y = 0; y < board_height; y++)
					{
						for (auto x = 0; x < board_width; x++)
						{
							auto id = c_main->cell(Vec2i(x, y));
							d[y * board_width + x] = '0' + id;
						}
					}
					req["board"] = d;
					auto str = req.dump();
					if (server)
					{
						for (auto i = 1; i < players.size(); i++)
						{
							auto& p = players[i];
							if (p.id && !p.disconnected)
								server->send(p.id, str.data(), str.size(), false);
						}
					}
					if (client)
						client->send(str.data(), str.size());

					if (dead)
					{
						if (server)
							people_dead(my_room_index);
						if (client)
						{
							nlohmann::json req;
							req["action"] = "report_dead";
							auto str = req.dump();
							client->send(str.data(), str.size());
						}
					}
				}
			}

			update_status();
			if (e_garbage->child_count() != garbages.size())
			{
				e_garbage->remove_children(0, -1);
				ui::push_parent(e_garbage);
				for (auto i = 0; i < garbages.size(); i++)
				{
					ui::next_element_pos = Vec2f(0.f, -i * 24.f);
					ui::next_element_size = Vec2f(24.f);
					ui::e_image((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH("gray.png")));
				}
				ui::pop_parent();
			}
			for (auto i = 0; i < garbages.size(); i++)
			{
				if (garbages[i] > 0)
					garbages[i]--;
				if (garbages[i] == 0)
					e_garbage->child(i)->get_component(cImage)->color = Vec4c(255, 0, 0, 255);
			}
		}
	}
}app;

int main(int argc, char **args)
{
	std::filesystem::path resource_path;
	std::filesystem::path engine_path;
	{
		auto config = parse_ini_file(L"config.ini");
		for (auto& e : config.get_section_entries(""))
		{
			if (e.key == "resource_path")
				resource_path = e.value;
			else if (e.key == "engine_path")
			{
				if (e.value == "{e}")
					engine_path = getenv("FLAME_PATH");
				else
					engine_path = e.value;
			}
		}
	}

	app.create("Tetris", Vec2u(800, 600), WindowFrame, false, engine_path);

	app.atlas = graphics::Atlas::load(app.graphics_device, (resource_path / L"art/atlas/main.atlas").c_str());
	app.canvas->add_atlas(app.atlas);

	{
		app.sound_move_buf = sound::Buffer::create_from_file((resource_path / L"art/move.wav").c_str());
		app.sound_move_src = sound::Source::create(app.sound_move_buf);
		app.sound_move_src->set_volume(sound_move_volumn);
	}
	{
		app.sound_soft_drop_buf = sound::Buffer::create_from_file((resource_path / L"art/soft_drop.wav").c_str());
		app.sound_soft_drop_src = sound::Source::create(app.sound_soft_drop_buf);
		app.sound_soft_drop_src->set_volume(sound_soft_drop_volumn);
	}
	{
		app.sound_hard_drop_buf = sound::Buffer::create_from_file((resource_path / L"art/hard_drop.wav").c_str());
		app.sound_hard_drop_src = sound::Source::create(app.sound_hard_drop_buf);
		app.sound_hard_drop_src->set_volume(sound_hard_drop_volumn);
	}
	{
		app.sound_clear_buf = sound::Buffer::create_from_file((resource_path / L"art/clear.wav").c_str());
		app.sound_clear_src = sound::Source::create(app.sound_clear_buf);
		app.sound_clear_src->set_volume(sound_clear_volumn);
	}
	{
		app.sound_hold_buf = sound::Buffer::create_from_file((resource_path / L"art/hold.wav").c_str());
		app.sound_hold_src = sound::Source::create(app.sound_hold_buf);
		app.sound_hold_src->set_volume(sound_hold_volumn);
	}

	ui::set_current_entity(app.root);
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::set_current_root(app.root);

	ui::push_parent(app.root);
	{
		auto e = ui::e_text(L"");
		e->on_destroyed_listeners.add([](void* c) {
			looper().remove_event(*(void**)c);
			return true;
		}, new_mail_p(add_fps_listener([](void* c, uint fps) {
			(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
		}, new_mail_p(e->get_component(cText)))));
	}
	ui::c_aligner(AlignxLeft, AlignyBottom);
	ui::pop_parent();

	app.create_home_scene();

	auto user_data = parse_ini_file(L"user_data.ini");
	for (auto& e : user_data.get_section_entries(""))
	{
		if (e.key == "name")
			app.my_name = s2w(e.value);
	}
	auto key_info = find_enum(FLAME_CHASH("flame::Key"));
	for (auto& e : user_data.get_section_entries("key"))
	{
		for (auto i = 0; i < KEY_COUNT; i++)
		{
			if (key_names[i] == s2w(e.key))
				key_map[i] = (Key)key_info->find_item(e.value.c_str())->value();
		}
	}
	for (auto& e : user_data.get_section_entries("sound"))
	{
		if (e.key == "fx_volumn")
			fx_volumn = std::stoi(e.value);
	}
	for (auto& e : user_data.get_section_entries("sensitiveness"))
	{
		if (e.key == "left_right_sensitiveness")
			left_right_sensitiveness = std::stoi(e.value);
		else if (e.key == "left_right_speed")
			left_right_speed = std::stoi(e.value);
		else if (e.key == "soft_drop_speed")
			soft_drop_speed = std::stoi(e.value);
	}

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
