#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/network/network.h>
#include <flame/utils/app.h>

#include "mino.h"
#include "key.h"

using namespace flame;

const auto board_width = 10U;
const auto board_height = 24U;
const auto DOWN_TICKS = 24U;
const auto CLEAR_TICKS = 6U;

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
	GameMenu,
	GameSingleMarathon,
	GameSingleRTA,
	GameSinglePractice,
	GameMulti
};

enum RoomState
{
	RoomPrepare,
	RoomGaming
};

struct RoomInfo
{
	std::wstring name;
	std::wstring host;
	std::string ip;
};

struct Player
{
	std::wstring name;
	void* id;
};

struct MyApp : App
{
	std::wstring your_name;
	std::wstring room_name;
	RoomState room_state;
	std::vector<RoomInfo> rooms;
	std::vector<Player> players;
	Server* server;
	Client* client;

	sEventDispatcher* s_event_dispatcher;
	graphics::Atlas* atlas;

	GameMode game_mode;

	Entity* e_base;
	cTileMap* c_board_main;
	cTileMap* c_board_hold;
	cTileMap* c_board_next[6];
	cTileMap* c_board_opponent_main;
	cText* c_text_time;
	cText* c_text_level;
	cText* c_text_lines;
	cText* c_text_score;
	cText* c_text_garage;

	bool just_down_rotate_left;
	bool just_down_rotate_right;
	bool just_down_hard_drop;
	bool just_down_hold;
	int left_frames;
	int right_frames;

	bool paused;
	bool gaming;
	bool win;
	float time;
	float play_time;
	uint level;
	uint lines;
	uint score;
	int clear_ticks;
	uint full_lines[4];
	uint garbage;
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
						app.rooms.clear();
						 
						auto e_room_list = *(Entity**)c;
						e_room_list->remove_children(0, -1);
						nlohmann::json req;
						req["action"] = "get_room";
						auto str = req.dump();
						board_cast(2434, str.data(), str.size(), 1, [](void* c, const char* ip, const char* msg, uint size) {
							auto e_room_list = *(Entity**)c;
							auto reply = nlohmann::json::parse(std::string(msg, size));
							RoomInfo info;
							info.name = s2w(reply["name"].get<std::string>());
							info.host = s2w(reply["host"].get<std::string>());
							info.ip = ip;
							app.rooms.push_back(info);

							ui::push_parent(e_room_list);
							ui::e_list_item((L"Name:" + info.name + L" Host:" + info.host).c_str());
							ui::pop_parent();
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
									if (app.room_state == RoomPrepare)
									{
										app.server->set_client(id,
										[](void* c, const char* msg, uint size) {
											auto id = *(void**)c;
											for (auto i = 0; i < app.players.size(); i++)
											{
												auto& p = app.players[i];
												if (p.id == id)
												{
													auto req = nlohmann::json::parse(std::string(msg, size));
													auto action = req["action"].get<std::string>();
													if (action == "report_name")
													{
														p.name = s2w(req["name"].get<std::string>());

														looper().add_event([](void* c, bool*) {
															ui::push_parent(app.e_base);
																ui::push_style_1u(ui::FontSize, 30);
																ui::next_element_pos = Vec2f(500.f, 10.f);
																ui::e_text(app.players[*(int*)c].name.c_str());
																ui::pop_style(ui::FontSize);
															ui::pop_parent();
														}, new_mail(&i));
													}
													else if (action == "report_board")
													{
														auto board = req["board"].get<std::string>();
														looper().add_event([](void* c, bool*) {
															auto& board = *(std::string*)c;
															for (auto y = 0; y < board_height; y++)
															{
																for (auto x = 0; x < board_width; x++)
																{
																	auto id = board[y * board_width + x] - '0';
																	app.c_board_opponent_main->set_cell(Vec2u(x, y), id, id == TileGrid ? Vec4c(255) : Vec4c(200, 200, 200, 255));
																}
															}
														}, new_mail(&board));
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
														}, new_mail(&value));
													}

													break;
												}
											}
										},
										[](void*) {
										}, new_mail_p(id));

										Player player;
										player.id = id;
										app.players.push_back(player);

										app.room_state = RoomGaming;
										ui::push_parent(app.e_base);
										app.c_board_opponent_main = app.create_board_main(Vec2f(500.f, 50.f), 24.f);
										ui::pop_parent();

										app.start_game();
									}
								}, Mail<>());
								looper().add_event([](void*, bool*) {
									app.rooms.clear();

									app.root->remove_children(1, -1);
									app.game_mode = GameMulti;
									app.create_game_scene();
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
							auto room_id = e_room_list->find_child(selected);
							auto& room = app.rooms[room_id];
							app.client = Client::create(SocketNormal, room.ip.c_str(), 2434,
							[](void*, const char* msg, uint size) {
								auto req = nlohmann::json::parse(std::string(msg, size));
								auto action = req["action"].get<std::string>();
								if (action == "report_board")
								{
									auto board = req["board"].get<std::string>();
									looper().add_event([](void* c, bool*) {
										auto& board = *(std::string*)c;
										for (auto y = 0; y < board_height; y++)
										{
											for (auto x = 0; x < board_width; x++)
											{
												auto id = board[y * board_width + x] - '0';
												app.c_board_opponent_main->set_cell(Vec2u(x, y), id, id == TileGrid ? Vec4c(255) : Vec4c(200, 200, 200, 255));
											}
										}
									}, new_mail(&board));
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
									}, new_mail(&value));
								}
							}, 
							[](void*) {
							}, Mail<>());
							if (app.client)
							{
								app.room_name = room.name;
								nlohmann::json req;
								req["action"] = "report_name";
								req["name"] = w2s(app.your_name);
								auto str = req.dump();
								app.client->send(str.data(), str.size());
								looper().add_event([](void* c, bool*) {
									app.root->remove_children(1, -1);
									app.game_mode = GameMulti;
									app.create_game_scene();
									ui::push_parent(app.e_base);
									app.c_board_opponent_main = app.create_board_main(Vec2f(500.f, 50.f), 24.f);
									ui::next_element_pos = Vec2f(500.f, 10.f);
									ui::push_style_1u(ui::FontSize, 30);
									ui::e_text(app.rooms[*(int*)c].host.c_str());
									ui::pop_style(ui::FontSize);
									ui::pop_parent();
									app.rooms.clear();

									app.start_game();
								}, new_mail(&room_id));
							}
						}
					}
				}, new_mail_p(e_room_list));
				ui::e_button(L"Back", [](void*) {
					looper().add_event([](void*, bool*) {
						app.rooms.clear();

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

	cTileMap* create_board_main(const Vec2f& pos, float block_size)
	{
		cTileMap* ret;
		ui::e_empty();
		ui::next_element_pos = pos;
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
			ret = cTileMap::create();
			ret->cell_size_ = Vec2f(block_size);
			ret->set_size(Vec2u(board_width, board_height));
			ret->clear_cells(TileGrid);
			set_board_tiles(ret);
			ui::current_entity()->add_component(ret);
		}
		ui::pop_parent();
		return ret;
	}

	void create_game_scene()
	{
		ui::push_parent(root);
		ui::push_style_1u(ui::FontSize, 20);

		if (game_mode == GameMulti)
			ui::e_text(room_name.c_str());

			ui::next_element_pos = Vec2f(game_mode == GameMulti ? -20.f : 100.f, 30.f);
			e_base = ui::e_element();
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
					ui::e_text(your_name.c_str());
					break;
				}
				ui::pop_style(ui::FontSize);

				auto block_size = 24.f;

				c_board_main = create_board_main(Vec2f(120.f, 50.f), block_size);

				block_size = 16.f;

				ui::next_element_pos = Vec2f(52.f, 40.f);
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
					c_board_hold->set_size(Vec2u(4));
					set_board_tiles(c_board_hold);
					ui::current_entity()->add_component(c_board_hold);
				}

				ui::next_element_pos = Vec2f(375.f, 40.f);
				ui::e_text(L"Next");

				ui::e_empty();
				ui::next_element_pos = Vec2f(370.f, 70.f);
				ui::next_element_size = Vec2f(block_size * 4 + 8.f, 52.f * array_size(c_board_next) + 8.f);
				{
					auto ce = ui::c_element();
					ce->color_ = Vec4c(30, 30, 30, 255);
				}
				for (auto i = 0; i < array_size(c_board_next); i++)
				{
					ui::e_empty();
					ui::next_element_pos = Vec2f(370.f, 70.f + 52.f * i);
					ui::next_element_size = Vec2f(block_size * 4 + 8.f);
					ui::c_element()->inner_padding_ = Vec4f(4.f);
					{
						c_board_next[i] = cTileMap::create();
						c_board_next[i]->cell_size_ = Vec2f(block_size);
						c_board_next[i]->set_size(Vec2u(4));
						set_board_tiles(c_board_next[i]);
						ui::current_entity()->add_component(c_board_next[i]);
					}
				}

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
					ui::push_style_1u(ui::FontSize, 20);
					ui::next_element_pos = Vec2f(90.f, 500.f);
					c_text_garage = ui::e_text(L"")->get_component(cText);
					c_text_garage->color = Vec4c(200, 120, 100, 255);
					ui::pop_style(ui::FontSize);
				}

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
		else
			c_text_garage->set_text(wfmt(L"%02d", garbage).c_str());
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
		play_time = 0.f;
		level = 1;
		lines = 0;
		score = 0;
		clear_ticks = -1;
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

		if (app.server)
			Server::destroy(app.server);
		if (app.client)
			Client::destroy(app.client);
		app.players.clear();

		looper().add_event([](void*, bool*) {
			app.root->remove_children(1, -1);
			app.game_mode = GameMenu;
			app.create_home_scene();
		}, Mail<>());
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

		if (game_mode != GameMenu)
		{
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

			auto dt = looper().delta_time;
			time += dt;
			if (!paused && gaming)
				play_time += dt;
			const auto frame_rate = 1.f / 24.f;
			while (time > frame_rate)
			{
				if ((game_mode == GameMulti || !paused) && gaming && !win)
				{
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
												c_board_main->set_cell(Vec2u(x, j), id, id == TileGrid ? Vec4c(255) : Vec4c(200, 200, 200, 255));
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
									server->send(players[0].id, str.data(), str.size(), false);
								if (client)
									client->send(str.data(), str.size());
							}

							auto gameover = !line_empty(3);
							if (!gameover)
							{
								if (game_mode == GameSingleRTA && lines >= 40)
									gameover = true;
							}
							if (gameover)
							{
								gaming = false;

								if (game_mode == GameMulti)
								{
									nlohmann::json req;
									req["action"] = "report_gameover";
									auto str = req.dump();
									if (server)
										server->send(players[0].id, str.data(), str.size(), false);
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

							clear_ticks = -1;
						}
					}
					else
					{
						if (mino_pos.y() != -1)
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
							mino_pos = Vec2i(4, 3);
							mino_rotation = 0;
							for (auto i = 0; i < 3; i++)
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
											mino_ticks = DOWN_TICKS;
										else
											mino_ticks = 0;
										mino_reset_times++;
									}
								}
							}
							auto is_soft_drop = key_states[key_map[KEY_SOFT_DROP]] & KeyStateDown;
							auto down_ticks_final = DOWN_TICKS;
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
									draw_mino(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords, Vec4c(200, 200, 200, 255));

									for (auto i = 0; i < 4; i++)
										full_lines[i] = -1;
									auto l = 0;
									for (auto i = 0; i < board_height; i++)
									{
										if (line_full(i))
										{
											for (auto x = 0; x < board_width; x++)
												c_board_main->set_cell(Vec2u(x, i), TileGrid);
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
												capture.f = 10;
												looper().add_event([](void* c, bool* go_on) {
													auto& capture = *(Capture*)c;
													capture.f--;
													if (capture.f > 0)
													{
														capture.e->pos_.x() -= 5.f;
														capture.e->size_.x() += 10.f;
														capture.e->pos_.y() += 1.2f;
														capture.e->size_.y() -= 2.4f;
														capture.e->color_.a() = max(capture.e->color_.a() - 15, 0);
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
										clear_ticks = CLEAR_TICKS;
										if (game_mode == GameSingleMarathon && lines % 5 == 0)
										{
											level++;
											level = min(24U, level);
										}

										auto attack = 0;
										switch (l)
										{
										case 1:
											score += 100;
											break;
										case 2:
											score += 300;
											attack = 1;
											break;
										case 3:
											score += 500;
											attack = 2;
											break;
										case 4:
											score += 800;
											attack = 4;
											break;
										}

										if (attack > 0)
										{
											nlohmann::json req;
											req["action"] = "attack";
											req["value"] = attack;
											auto str = req.dump();
											if (server)
												server->send(players[0].id, str.data(), str.size(), false);
											if (client)
												client->send(str.data(), str.size());
										}
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
													c_board_main->set_cell(Vec2u(x, i), id, id == TileGrid ? Vec4c(255) : Vec4c(200, 200, 200, 255));
												}
											}
											auto hole = ::rand() % board_width;
											for (auto i = 0; i < garbage; i++)
											{
												auto y = board_height - i - 1;
												for (auto x = 0; x < board_width; x++)
													c_board_main->set_cell(Vec2u(x, y), TileGray, Vec4c(200, 200, 200, 255));
												c_board_main->set_cell(Vec2u(hole, y), TileGrid);
											}
											garbage = 0;
										}

										clear_ticks = 0;
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
									draw_mino(c_board_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords);
								draw_mino(c_board_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
							}
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
	}
}app;

int main(int argc, char **args)
{
	app.create("Tetris", Vec2u(800, 600), WindowFrame);

	auto w = app.u->world(0);

	app.s_event_dispatcher = w->get_system(sEventDispatcher);

	app.atlas = graphics::Atlas::load(app.d, L"../game/tetris/art/atlas/main.atlas");
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

	app.game_mode = GameMenu;
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
