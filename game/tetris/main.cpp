#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/universe/components/data_keeper.h>
#include <flame/network/network.h>
#include <flame/utils/app.h>
#include <flame/universe/ui/typeinfo_utils.h>

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
	std::wstring name;
	void* id;

	Entity* e;
	cTileMap* c_board;

	Player() :
		id(nullptr)
	{
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
	Server* server;
	uint room_max_people;
	Client* client;

	GameMode game_mode;

	Entity* e_base;
	cTileMap* c_board_main;
	cTileMap* c_board_hold;
	cTileMap* c_board_next[6];
	cText* c_text_time;
	cText* c_text_level;
	cText* c_text_lines;
	cText* c_text_score;
	cText* c_text_special;
	Entity* e_garbage;

	int left_frames;
	int right_frames;

	bool paused;
	bool gaming;
	bool win;
	float play_time;
	uint level;
	uint lines;
	uint score;
	int clear_ticks;
	uint full_lines[4];
	uint combo;
	bool back_to_back;
	uint garbage;
	bool need_update_garbage;
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
				app.my_room_index = req["index"].get<int>();
				app.root->remove_children(1, -1);
				app.game_mode = GameMulti;
				app.create_game_scene();
			}
			else if (action == "player_entered")
			{
				auto& p = app.players[req["index"].get<int>()];
				p.name = s2w(req["name"].get<std::string>());

				ui::push_parent(app.e_base);
				p.c_board = app.create_board_main(Vec2f(500.f, 50.f), 24.f, p.name.c_str());
				ui::pop_parent();
			}
			else if(action == "game_start")
				app.start_game();
			else if (action == "report_board")
			{
				struct Capture
				{
					cTileMap* b;
					std::string d;
				}capture;
				capture.b = app.players[req["index"].get<int>()].c_board;
				capture.d = req["board"].get<std::string>();
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
			else if (action == "report_gameover")
			{
				looper().add_event([](void* c, bool*) {
					app.win = true;
					if (app.paused)
						ui::remove_top_layer(app.root);
					ui::e_begin_dialog();
					ui::e_text(L"You Win");
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Quit", [](void*) {
						ui::remove_top_layer(app.root);

						app.quit_game();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_end_dialog();
				}, Mail<>());
			}
			else if (action == "attack")
			{
				auto value = req["value"].get<int>();
				looper().add_event([](void* c, bool*) {
					app.garbage = *(int*)c;
					app.need_update_garbage = true;
				}, new_mail(&value));
			}
		},
		[](void*) {
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
							auto reply = nlohmann::json::parse(std::string(msg, size));
							auto name = s2w(reply["name"].get<std::string>());
							auto host = s2w(reply["host"].get<std::string>());

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
										app.room_max_people = 6;
										break;
									}
								}
								return true;
							}, Mail<>());
							ui::e_combobox_item(L"2");
							ui::e_combobox_item(L"6");
							ui::e_end_combobox(0);
							app.room_max_people = 2;
							ui::e_begin_layout(LayoutHorizontal, 4.f);
							ui::c_aligner(AlignxMiddle, AlignyFree);
								ui::e_button(L"OK", [](void* c) {
									ui::remove_top_layer(app.root);

									if (!app.room_name.empty())
									{
										app.players.resize(app.room_max_people);
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
												nlohmann::json reply;
												reply["name"] = w2s(app.room_name);
												reply["host"] = w2s(app.my_name);
												auto str = reply.dump();
												app.server->send(id, str.data(), str.size(), true);
											}
										},
										[](void*, void* id) {
											if (!app.gaming)
											{
												for (auto i = 0; i < app.players.size(); i++)
												{
													auto& p = app.players[i];
													if (!p.id)
													{
														{
															nlohmann::json reply;
															reply["action"] = "report_room";
															reply["room_name"] = w2s(app.room_name);
															reply["max_people"] = app.room_max_people;
															reply["index"] = i;
															auto str = reply.dump();
															app.server->send(id, str.data(), str.size(), false);
														}
														for (auto j = 0; j < app.players.size(); j++)
														{
															auto& p = app.players[j];
															if (p.id)
															{
																nlohmann::json reply;
																reply["action"] = "player_entered";
																reply["index"] = j;
																reply["name"] = w2s(p.name);
																auto str = reply.dump();
																app.server->send(id, str.data(), str.size(), false);
															}
														}

														p.id = id;
														app.server->set_client(id,
														[](void* c, const char* msg, uint size) {
															auto& p = app.players[*(int*)c];
															auto req = nlohmann::json::parse(std::string(msg, size));
															auto action = req["action"].get<std::string>();
															if (action == "join_room")
															{
																p.name = s2w(req["name"].get<std::string>());

																ui::push_parent(app.e_base);
																p.c_board = app.create_board_main(Vec2f(500.f, 50.f), 24.f, p.name.c_str());
																ui::pop_parent();

																{
																	nlohmann::json reply;
																	reply["action"] = "game_start";
																	auto str = reply.dump();
																	app.server->send(p.id, str.data(), str.size(), false);
																}

																app.start_game();
															}
															else if (action == "report_board")
															{
																struct Capture
																{
																	cTileMap* b;
																	std::string d;
																}capture;
																capture.b = app.players[req["index"].get<int>()].c_board;
																capture.d = req["board"].get<std::string>();
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
															else if (action == "report_gameover")
															{
																looper().add_event([](void* c, bool*) {
																	app.win = true;
																	if (app.paused)
																	{
																		app.paused = false;
																		ui::remove_top_layer(app.root);
																	}
																	ui::e_begin_dialog();
																	ui::e_text(L"You Win");
																	ui::c_aligner(AlignxMiddle, AlignyFree);
																	ui::e_button(L"Quit", [](void*) {
																		ui::remove_top_layer(app.root);

																		app.quit_game();
																	}, Mail<>());
																	ui::c_aligner(AlignxMiddle, AlignyFree);
																	ui::e_end_dialog();
																}, Mail<>());
															}
															else if (action == "attack")
															{
																auto value = req["value"].get<int>();
																looper().add_event([](void* c, bool*) {
																	app.garbage = *(int*)c;
																	app.need_update_garbage = true;
																}, new_mail(&value));
															}
														},
														[](void*) {
														}, new_mail(&i));

														break;
													}
												}
											}
										}, Mail<>());
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

	cTileMap* create_board_main(const Vec2f& pos, float block_size, const wchar_t* title = nullptr)
	{
		if (title)
		{
			ui::push_style_1u(ui::FontSize, 30);
			ui::next_element_pos = Vec2f(pos.x(), pos.y() - 40.f);
			ui::e_text(title);
			ui::pop_style(ui::FontSize);
		}
		ui::e_empty();
		ui::next_element_pos = pos;
		ui::next_element_size = Vec2f(block_size * board_width, block_size * (board_height - 3.8f));
		{
			auto ce = ui::c_element();
			ce->frame_thickness_ = 6.f;
			ce->color_ = Vec4c(30, 30, 30, 255);
			ce->frame_color_ = Vec4c(255);
			ce->clip_children = true;
		}
		ui::push_parent(ui::current_entity());
			ui::e_empty();
			ui::next_element_pos = Vec2f(0.f, -block_size * 3.8f);
			ui::next_element_size = Vec2f(block_size * board_width, block_size * board_height);
			ui::c_element();
			auto board = cTileMap::create();
			board->cell_size_ = Vec2f(block_size);
			board->set_size(Vec2u(board_width, board_height));
			board->clear_cells(TileGrid);
			set_board_tiles(board);
			ui::current_entity()->add_component(board);
		ui::pop_parent();
		return board;
	}

	void create_game_scene()
	{
		ui::push_parent(root);
		ui::push_style_1u(ui::FontSize, 20);

			if (game_mode == GameMulti)
				ui::e_text(wfmt(L"Room: %s", room_name.c_str()).c_str());

			ui::next_element_pos = Vec2f(game_mode == GameMulti ? -20.f : 100.f, 30.f);
			e_base = ui::e_element();
			e_base->associate_resource(looper().add_event([](void*, bool* go_on) {
				app.do_game_logic();
				*go_on = true;
			}, new_mail_p(ui::current_entity()->get_component(cText))), [](void* ev) {
				looper().remove_event(ev);
			});
			ui::push_parent(e_base);

				ui::next_element_pos = Vec2f(120.f, 10.f);
				ui::push_style_1u(ui::FontSize, 30);
				switch (game_mode)
				{
				case GameSingleMarathon:
					ui::e_text(L"Marathon");
					break;
				case GameSingleRTA:
					ui::e_text(L"RTA");
					break;
				case GameSinglePractice:
					ui::e_text(L"Practice");
					break;
				case GameMulti:
					ui::e_text(my_name.c_str());
					break;
				}
				ui::pop_style(ui::FontSize);

				auto block_size = 24.f;

				c_board_main = create_board_main(Vec2f(120.f, 50.f), block_size);

				block_size = 16.f;

				ui::next_element_pos = Vec2f(52.f, 50.f);
				ui::e_text(L"Hold");

				ui::e_empty();
				ui::next_element_pos = Vec2f(37.f, 70.f);
				ui::next_element_size = Vec2f(block_size * 4 + 8.f);
				{
					auto ce = ui::c_element();
					ce->inner_padding_ = Vec4f(4.f);
					ce->color_ = Vec4c(30, 30, 30, 255);
				}
				{
					c_board_hold = cTileMap::create();
					c_board_hold->cell_size_ = Vec2f(block_size);
					c_board_hold->set_size(Vec2u(4, 3));
					set_board_tiles(c_board_hold);
					ui::current_entity()->add_component(c_board_hold);
				}

				ui::next_element_pos = Vec2f(375.f, 50.f);
				ui::e_text(L"Next");

				ui::e_empty();
				ui::next_element_pos = Vec2f(370.f, 70.f);
				ui::next_element_size = Vec2f(block_size * 4 + 8.f, (block_size * 3.f + 4.f) * array_size(c_board_next) + 8.f - 45.f);
				{
					auto ce = ui::c_element();
					ce->color_ = Vec4c(30, 30, 30, 255);
				}
				auto create_next_board = [&](int i, int base, float y_off, float block_size) {
					ui::e_empty();
					ui::next_element_pos = Vec2f(370.f, 70.f + y_off + (block_size * 3.f + 4.f) * (i - base));
					ui::next_element_size = Vec2f(block_size * 4 + 8.f);
					ui::c_element()->inner_padding_ = Vec4f(4.f);
					{
						c_board_next[i] = cTileMap::create();
						c_board_next[i]->cell_size_ = Vec2f(block_size);
						c_board_next[i]->set_size(Vec2u(4));
						set_board_tiles(c_board_next[i]);
						ui::current_entity()->add_component(c_board_next[i]);
					}
				};
				for (auto i = 0; i < 1; i++)
					create_next_board(i, 0, 0.f, 16.f);
				for (auto i = 1; i < 3; i++)
					create_next_board(i, 1, 16.f * 3.f + 4.f, 14.f);
				for (auto i = 3; i < array_size(c_board_next); i++)
					create_next_board(i, 3, 16.f * 3.f + 4.f + (14.f * 3.f + 4.f) * 2, 12.f);

				if (game_mode != GameMulti)
				{
					ui::next_element_pos = Vec2f(450.f, 120.f);
					ui::e_text(L"TIME")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(450.f, 180.f);
					ui::e_text(L"LEVEL")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(450.f, 240.f);
					ui::e_text(game_mode == GameSingleRTA ? L"LEFT" : L"LINES")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::next_element_pos = Vec2f(450.f, 300.f);
					ui::e_text(L"SCORE")->get_component(cText)->color = Vec4c(40, 80, 200, 255);

					ui::push_style_1u(ui::FontSize, 40);
					ui::next_element_pos = Vec2f(450.f, 140.f);
					c_text_time = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(450.f, 200.f);
					c_text_level = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(450.f, 260.f);
					c_text_lines = ui::e_text(L"")->get_component(cText);
					ui::next_element_pos = Vec2f(450.f, 320.f);
					c_text_score = ui::e_text(L"")->get_component(cText);
					ui::pop_style(ui::FontSize);
				}
				else
				{
					ui::next_element_pos = Vec2f(94.f, 516.f);
					e_garbage = ui::e_element();
				}

				ui::push_style_1u(ui::FontSize, 28);
				ui::next_element_pos = Vec2f(37.f, 200.f);
				{
					auto e = ui::e_text(L"");
					e->set_visibility(false);
					c_text_special = e->get_component(cText);
				}
				c_text_special->color = Vec4c(200, 80, 40, 255);
				ui::pop_style(ui::FontSize);

			ui::pop_parent();

		ui::pop_style(ui::FontSize);
		ui::pop_parent();
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
				app.gaming = true;
			}
		};
		auto capture = new_mail<Capture>();
		capture.p->time = 3;
		ui::push_parent(e_base);
		ui::push_style_1u(ui::FontSize, 80);
		ui::next_element_pos = Vec2f(220.f, 250.f);
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
		c_board_main->clear_cells(TileGrid);
		c_board_hold->clear_cells(-1);
		for (auto i = 0; i < array_size(c_board_next); i++)
			c_board_next[i]->clear_cells(-1);

		left_frames = -1;
		right_frames = -1;

		play_time = 0.f;
		level = 1;
		lines = 0;
		score = 0;
		clear_ticks = -1;
		combo = 0;
		back_to_back = false;
		garbage = 0;
		need_update_garbage = false;
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

		paused = false;
		gaming = false;
		win = false;
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

	void draw_mino(cTileMap* board, int idx, const Vec2i& pos, uint offset_y, Vec2i* coords, const Vec4c& col = Vec4c(255))
	{
		board->set_cell(Vec2u(pos) + Vec2u(0, offset_y), idx, col);
		board->set_cell(Vec2u(pos + coords[0] + Vec2u(0, offset_y)), idx, col);
		board->set_cell(Vec2u(pos + coords[1] + Vec2u(0, offset_y)), idx, col);
		board->set_cell(Vec2u(pos + coords[2] + Vec2u(0, offset_y)), idx, col);
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

	void quit_game()
	{
		looper().clear_events(FLAME_CHASH("count_down"));

		gaming = false;

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
		app.players.clear();

		looper().add_event([](void*, bool*) {
			app.root->remove_children(1, -1);
			app.create_home_scene();
		}, Mail<>());
	}

	void do_game_logic()
	{
		auto& key_states = s_event_dispatcher->key_states;

		if (key_states[key_map[KEY_PAUSE]] == (KeyStateDown | KeyStateJust))
		{
			if (!paused)
			{
				if (game_mode != GameMulti)
				{
					looper().clear_events(FLAME_CHASH("count_down"));

					ui::e_begin_dialog();
					ui::e_text(L"Paused");
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Resume", [](void*) {
						app.paused = false;
						ui::remove_top_layer(app.root);

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

						app.quit_game();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_end_dialog();
				}
				else
				{
					ui::e_begin_dialog();
					ui::e_text(L"Paused");
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Resume", [](void*) {
						app.paused = false;
						ui::remove_top_layer(app.root);
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_button(L"Quit", [](void*) {
						app.paused = false;
						ui::remove_top_layer(app.root);

						app.quit_game();
					}, Mail<>());
					ui::c_aligner(AlignxMiddle, AlignyFree);
					ui::e_end_dialog();
				}

				paused = true;
				if (game_mode != GameMulti)
					gaming = false;
			}
		}

		if ((game_mode == GameMulti || !paused) && gaming && !win)
		{
			play_time += looper().delta_time;

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
										auto id = c_board_main->cell(Vec2i(x, j - 1));
										c_board_main->set_cell(Vec2u(x, j), id, id == TileGrid ? Vec4c(255) : mino_col_decay);
									}
								}
								else
								{
									for (auto x = 0; x < board_width; x++)
										c_board_main->set_cell(Vec2u(x, j), TileGrid);
								}
							}
						}
					}

					if (game_mode == GameMulti)
					{
						nlohmann::json req;
						req["action"] = "report_board";
						req["index"] = my_room_index;
						std::string board;
						board.resize(board_height * board_width);
						for (auto y = 0; y < board_height; y++)
						{
							for (auto x = 0; x < board_width; x++)
							{
								auto id = c_board_main->cell(Vec2i(x, y));
								board[y * board_width + x] = '0' + id;
							}
						}
						req["board"] = board;
						auto str = req.dump();
						if (server)
						{
							for (auto i = 1; i < players.size(); i++)
								server->send(players[i].id, str.data(), str.size(), false);
						}
						if (client)
							client->send(str.data(), str.size());
					}

					clear_ticks = -1;
				}
			}
			else
			{
				if (mino_pos.y() >= 0)
				{
					draw_mino(c_board_main, TileGrid, mino_pos, 0, mino_coords);
					if (mino_bottom_dist > 0)
						draw_mino(c_board_main, TileGrid, mino_pos, mino_bottom_dist, mino_coords);
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
							draw_mino(c_board_next[i], TileMino1 + t, Vec2i(1), 0, coords);
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
							draw_mino(c_board_hold, TileMino1 + mino_hold, Vec2i(1), 0, coords);
						}
					}
					mino_pos = Vec2i(4, 5 - (mino_type == Mino_I ? 1 : 0));
					mino_rotation = 0;
					for (auto i = 0; i < 3; i++)
						mino_coords[i] = g_mino_coords[mino_type][i];
					mino_reset_times = -1;
					mino_ticks = 0;

					auto dead = !check_board(Vec2i(0));
					if (dead)
					{
						{
							auto pos = mino_pos;
							c_board_main->set_cell(Vec2u(pos), 
								c_board_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[0];
							c_board_main->set_cell(Vec2u(pos),
								c_board_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[1];
							c_board_main->set_cell(Vec2u(pos),
								c_board_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
						{
							auto pos = mino_pos + mino_coords[2];
							c_board_main->set_cell(Vec2u(pos),
								c_board_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
						}
					}
					if (dead || (game_mode == GameSingleRTA && lines >= 40))
					{
						gaming = false;

						if (game_mode == GameMulti)
						{
							nlohmann::json req;
							req["action"] = "report_gameover";
							auto str = req.dump();
							if (server)
							{
								for (auto i = 0; i < players.size(); i++)
									server->send(players[i].id, str.data(), str.size(), false);
							}
							if (client)
								client->send(str.data(), str.size());

							ui::e_begin_dialog();
							ui::e_text(L"You Lose");
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

				if (gaming)
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
							if (super_rotation(r == 1, new_coords, &offset))
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
						if (mx != 0 && check_board(Vec2i(mx, 0)))
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
						while (check_board(Vec2i(0, mino_bottom_dist + 1)))
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
								draw_mino(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords, mino_col_decay);

								for (auto i = 0; i < 4; i++)
									full_lines[i] = -1;
								auto l = 0U;
								for (auto i = 0; i < board_height; i++)
								{
									if (line_full(i))
									{
										full_lines[l] = i;

										{
											auto cell_size = c_board_main->cell_size_;
											auto board_element = c_board_main->element;
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

									auto attack = 0U;

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
												c_board_main->cell(p) != TileGrid)
												count++;
										}
										if (count < 3)
											tspin = false;
									}

									get_lines_award(l, tspin, mini, back_to_back, score, attack, special_str);

									auto cancel = max(attack, l);
									if (garbage)
									{
										if (garbage < cancel)
											garbage = 0;
										else
											garbage -= cancel;
										attack -= cancel;
										need_update_garbage = true;
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
												server->send(players[i].id, str.data(), str.size(), false);
										}
										if (client)
											client->send(str.data(), str.size());
									}

									for (auto i = 0; i < l; i++)
									{
										for (auto x = 0; x < board_width; x++)
											c_board_main->set_cell(Vec2u(x, full_lines[i]), TileGrid);
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
									if (garbage)
									{
										for (auto i = 0; i < board_height - garbage; i++)
										{
											for (auto x = 0; x < board_width; x++)
											{
												auto id = c_board_main->cell(Vec2i(x, i + garbage));
												c_board_main->set_cell(Vec2u(x, i), id, id == TileGrid ? Vec4c(255) : mino_col_decay);
											}
										}
										auto hole = ::rand() % board_width;
										for (auto i = 0; i < garbage; i++)
										{
											auto y = board_height - i - 1;
											for (auto x = 0; x < board_width; x++)
												c_board_main->set_cell(Vec2u(x, y), TileGray, mino_col_decay);
											c_board_main->set_cell(Vec2u(hole, y), TileGrid);
										}
										garbage = 0;
										need_update_garbage = true;
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
								draw_mino(c_board_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords, g_mino_colors[mino_type]);
							draw_mino(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
						}
					}
				}
			}

			update_status();
			if (need_update_garbage)
			{
				e_garbage->remove_children(0, -1);
				ui::push_parent(e_garbage);
				for (auto i = 0; i < garbage; i++)
				{
					ui::next_element_pos = Vec2f(0.f, -i * 24.f);
					ui::next_element_size = Vec2f(24.f);
					ui::e_image((atlas->canvas_slot_ << 16) + atlas->find_tile(FLAME_HASH("gray.png")));
				}
				ui::pop_parent();
				need_update_garbage = false;
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
			if (e.key == "debug")
				set_debug_config(e.value == "1");
			else if (e.key == "resource_path")
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
	ui::e_text(L"")->associate_resource(add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, new_mail_p(ui::current_entity()->get_component(cText))), [](void* ev) {
		looper().remove_event(ev);
	});
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
