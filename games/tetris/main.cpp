#include <flame/serialize.h>
#include <flame/universe/app.h>
#include <flame/network/network.h>
#include <flame/universe/components/text.h>

using namespace flame;

enum MinoType
{
	Mino_L,
	Mino_T,
	Mino_S,
	Mino_Z,
	Mino_J,
	Mino_O,
	Mino_I,

	MinoTypeCount
};

ivec2 g_mino_coords[MinoTypeCount][3];
cvec4 g_mino_colors[MinoTypeCount];

ivec2 g_mino_LTSZJ_offsets[5][4];
ivec2 g_mino_O_offsets[5][4];
ivec2 g_mino_I_offsets[5][4];

enum
{
	KEY_PAUSE,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ROTATE_LEFT,
	KEY_ROTATE_RIGHT,
	KEY_SOFT_DROP,
	KEY_HARD_DROP,
	KEY_HOLD,
	KEY_COUNT
};


KeyboardKey key_map[KEY_COUNT];
const wchar_t* key_names[KEY_COUNT];

inline void get_combo_award(uint combo, int& attack, std::wstring& str)
{
	if (combo >= 1)
	{
		if (combo > 12)
			attack += 5;
		else
		{
			switch (combo)
			{
			case 2:
				attack += 1;
				break;
			case 3:
				attack += 1;
				break;
			case 4:
				attack += 2;
				break;
			case 5:
				attack += 2;
				break;
			case 6:
				attack += 3;
				break;
			case 7:
				attack += 3;
				break;
			case 8:
				attack += 4;
				break;
			case 9:
				attack += 4;
				break;
			case 10:
				attack += 4;
				break;
			case 11:
				attack += 5;
				break;
			case 12:
				attack += 5;
				break;
			}
		}
		str += wfmt(L"Ren %d\n", combo);
	}
}

inline void get_lines_award(uint l, bool tspin, bool mini, bool& back_to_back, uint& score, int& attack, std::wstring& str)
{
	switch (l)
	{
	case 1:
		if (tspin)
		{
			if (mini)
			{
				score += 400;
				str += L"T-Spin\nMini\n";
			}
			else
			{
				score += 800;
				attack += 2;
				str += L"T-Spin\nSingle\n";
			}
			if (back_to_back)
			{
				score += mini ? 200 : 400;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
			back_to_back = false;
		break;
	case 2:
		if (tspin)
		{
			score += 1200;
			attack += 4;
			str += L"T-Spin\nDouble\n";
			if (back_to_back)
			{
				score += 600;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
		{
			score += 300;
			attack += 1;
			str += L"Double\n";
			back_to_back = false;
		}
		break;
	case 3:
		if (tspin)
		{
			score += 1600;
			attack += 6;
			str += L"T-Spin\nTriple\n";
			if (back_to_back)
			{
				score += 800;
				attack++;
				str += L"\nBack\nTo\nBack";
			}
			back_to_back = true;
		}
		else
		{
			score += 500;
			attack += 2;
			str += L"Triple\n";
			back_to_back = false;
		}
		break;
	case 4:
		score += 800;
		attack += 4;

		str += L"Tetris\n";
		if (back_to_back)
		{
			score += 400;
			attack++;
			str += L"\nBack\nTo\nBack";
		}
		back_to_back = true;
		break;
	}
}


const auto board_width = 10U;
const auto board_height = 24U;
const auto DOWN_TICKS = 60U;
const auto CLEAR_TICKS = 15U;
const auto mino_col_decay = cvec4(200, 200, 200, 255);

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
	GameVS
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
	Entity* e_count_down;
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

struct Garbage
{
	uint time;
	uint lines;
};

struct MyApp : App
{
	graphics::ImageAtlas* atlas;

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
	uint my_room_index = 0;
	bool room_gaming;
	Server* server = nullptr;
	uint room_max_people;
	Client* client = nullptr;

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
	std::vector<Garbage> garbages;
	bool need_update_garbages_tip;
	ivec2 mino_pos;
	MinoType mino_type;
	MinoType mino_hold;
	bool mino_just_hold;
	uint mino_rotation;
	ivec2 mino_coords[3];
	int mino_reset_times;
	uint mino_bottom_dist;
	uint mino_ticks;
	uvec2 mino_pack_idx;
	MinoType mino_packs[2][MinoTypeCount];

	MyApp();
	~MyApp();
	void create();
	void create_home_scene();
	void create_player_controls(int player_index);
	void process_player_entered(int index);
	void process_player_disconnected(int index);
	void process_player_left(int index);
	void process_player_ready(int index);
	void process_game_start();
	void process_report_board(int index, std::string_view d);
	void process_attack(int index, int value);
	void process_dead(int index, int rank);
	void process_gameover();
	void join_room(const char* ip);
	void people_dead(int index);
	void create_lan_scene();
	void create_config_scene();
	void create_key_scene();
	void create_sound_scene();
	void create_sensitiveness_scene();
	void set_board_tiles(cTileMap* m);
	void create_game_scene();
	void do_game_logic();
	void begin_count_down();
	void update_status();
	void start_game();
	void shuffle_pack(uint idx);
	void draw_mino(cTileMap* board, int idx, const ivec2& pos, uint offset_y, ivec2* coords, const cvec4& col = cvec4(255));
	bool check_board(cTileMap* board, const ivec2& p);
	bool check_board(cTileMap* board, ivec2* in, const ivec2& p);
	bool line_empty(cTileMap* board, uint l);
	bool line_full(cTileMap* board, uint l);
	uint get_rotation_idx(bool clockwise);
	bool super_rotation(cTileMap* board, bool clockwise, ivec2* out_coord, ivec2* offset);
	void quit_game();
}g_app;

struct MainForm : GraphicsWindow
{
	MainForm();
};

MainForm::MainForm() :
	GraphicsWindow(&g_app, true, true, "Tetris", uvec2(800, 600), WindowFrame)
{
	canvas->set_resource(-1, g_app.atlas, "");

	{
		auto e = ui.e_text(L"");
		looper().add_event([](Capture& c) {
			c.thiz<cText>()->set_text(std::to_wstring(fps).c_str());
			c._current = nullptr;
		}, Capture().set_thiz(e->get_component<cText>()), 1.f);
	}
	ui.c_aligner(AlignMin, AlignMax);

	g_app.create_home_scene();
}

MyApp::MyApp()
{
	players.resize(1);
	players[0].id = (void*)0xffff;

	g_mino_coords[Mino_L][0] = ivec2(-1, +0);
	g_mino_coords[Mino_L][1] = ivec2(+1, +0);
	g_mino_coords[Mino_L][2] = ivec2(-1, -1);

	g_mino_coords[Mino_J][0] = ivec2(-1, +0);
	g_mino_coords[Mino_J][1] = ivec2(+1, +0);
	g_mino_coords[Mino_J][2] = ivec2(+1, -1);

	g_mino_coords[Mino_T][0] = ivec2(-1, +0);
	g_mino_coords[Mino_T][1] = ivec2(+1, +0);
	g_mino_coords[Mino_T][2] = ivec2(+0, -1);

	g_mino_coords[Mino_S][0] = ivec2(-1, +0);
	g_mino_coords[Mino_S][1] = ivec2(+0, -1);
	g_mino_coords[Mino_S][2] = ivec2(+1, -1);

	g_mino_coords[Mino_Z][0] = ivec2(-1, -1);
	g_mino_coords[Mino_Z][1] = ivec2(+0, -1);
	g_mino_coords[Mino_Z][2] = ivec2(+1, +0);

	g_mino_coords[Mino_O][0] = ivec2(+0, -1);
	g_mino_coords[Mino_O][1] = ivec2(+1, +0);
	g_mino_coords[Mino_O][2] = ivec2(+1, -1);

	g_mino_coords[Mino_I][0] = ivec2(-1, +0);
	g_mino_coords[Mino_I][1] = ivec2(+1, +0);
	g_mino_coords[Mino_I][2] = ivec2(+2, +0);

	g_mino_colors[Mino_L] = cvec4(0, 81, 179, 255);
	g_mino_colors[Mino_T] = cvec4(169, 0, 225, 255);
	g_mino_colors[Mino_S] = cvec4(0, 221, 50, 255);
	g_mino_colors[Mino_Z] = cvec4(193, 0, 0, 255);
	g_mino_colors[Mino_J] = cvec4(230, 132, 0, 255);
	g_mino_colors[Mino_O] = cvec4(225, 198, 0, 255);
	g_mino_colors[Mino_I] = cvec4(0, 184, 217, 255);

	memset(g_mino_LTSZJ_offsets, 0, sizeof(g_mino_LTSZJ_offsets));
	memset(g_mino_O_offsets, 0, sizeof(g_mino_O_offsets));
	memset(g_mino_I_offsets, 0, sizeof(g_mino_I_offsets));

	g_mino_LTSZJ_offsets[0][0] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[0][1] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[0][2] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[0][3] = ivec2(+0, +0);

	g_mino_LTSZJ_offsets[1][0] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[1][1] = ivec2(+1, +0);
	g_mino_LTSZJ_offsets[1][2] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[1][3] = ivec2(-1, +0);

	g_mino_LTSZJ_offsets[2][0] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[2][1] = ivec2(+1, +1);
	g_mino_LTSZJ_offsets[2][2] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[2][3] = ivec2(-1, +1);

	g_mino_LTSZJ_offsets[3][0] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[3][1] = ivec2(+0, -2);
	g_mino_LTSZJ_offsets[3][2] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[3][3] = ivec2(+0, -2);

	g_mino_LTSZJ_offsets[4][0] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[4][1] = ivec2(+1, -2);
	g_mino_LTSZJ_offsets[4][2] = ivec2(+0, +0);
	g_mino_LTSZJ_offsets[4][3] = ivec2(-1, -2);

	g_mino_I_offsets[0][0] = ivec2(+0, +0);
	g_mino_I_offsets[0][1] = ivec2(-1, +0);
	g_mino_I_offsets[0][2] = ivec2(-1, -1);
	g_mino_I_offsets[0][3] = ivec2(+0, -1);

	g_mino_I_offsets[1][0] = ivec2(-1, +0);
	g_mino_I_offsets[1][1] = ivec2(+0, +0);
	g_mino_I_offsets[1][2] = ivec2(+1, -1);
	g_mino_I_offsets[1][3] = ivec2(+0, -1);

	g_mino_I_offsets[2][0] = ivec2(+2, +0);
	g_mino_I_offsets[2][1] = ivec2(+0, +0);
	g_mino_I_offsets[2][2] = ivec2(-2, -1);
	g_mino_I_offsets[2][3] = ivec2(+0, -1);

	g_mino_I_offsets[3][0] = ivec2(-1, +0);
	g_mino_I_offsets[3][1] = ivec2(+0, -1);
	g_mino_I_offsets[3][2] = ivec2(+1, +0);
	g_mino_I_offsets[3][3] = ivec2(+0, +1);

	g_mino_I_offsets[4][0] = ivec2(+2, +0);
	g_mino_I_offsets[4][1] = ivec2(+0, +2);
	g_mino_I_offsets[4][2] = ivec2(-2, +0);
	g_mino_I_offsets[4][3] = ivec2(+0, -2);

	g_mino_O_offsets[0][0] = ivec2(+0, +0);
	g_mino_O_offsets[0][1] = ivec2(+0, +1);
	g_mino_O_offsets[0][2] = ivec2(-1, +1);
	g_mino_O_offsets[0][3] = ivec2(-1, +0);

	key_map[KEY_PAUSE] = Keyboard_Esc;
	key_map[KEY_LEFT] = Keyboard_A;
	key_map[KEY_RIGHT] = Keyboard_D;
	key_map[KEY_ROTATE_LEFT] = Keyboard_J;
	key_map[KEY_ROTATE_RIGHT] = Keyboard_K;
	key_map[KEY_SOFT_DROP] = Keyboard_S;
	key_map[KEY_HARD_DROP] = Keyboard_Space;
	key_map[KEY_HOLD] = Keyboard_E;

	key_names[KEY_PAUSE] = L"Pause";
	key_names[KEY_LEFT] = L"Left";
	key_names[KEY_RIGHT] = L"Right";
	key_names[KEY_ROTATE_LEFT] = L"Rotate_Left";
	key_names[KEY_ROTATE_RIGHT] = L"Rotate_Right";
	key_names[KEY_SOFT_DROP] = L"Soft_Drop";
	key_names[KEY_HARD_DROP] = L"Hard_Drop";
	key_names[KEY_HOLD] = L"Hold";
}

MyApp::~MyApp()
{
	std::ofstream user_data(L"user_data.ini");
	user_data << "name = " << w2s(my_name) << "\n";
	user_data << "\n[key]\n";
	auto key_info = find_enum("flame::KeyboardKey");
	for (auto i = 0; i < KEY_COUNT; i++)
		user_data << w2s(key_names[i]) << " = " << key_info->find_item(key_map[i])->get_name() << "\n";
	user_data << "\n[sound]\n";
	user_data << "fx_volumn = " << fx_volumn << "\n";
	user_data << "\n[sensitiveness]\n";
	user_data << "left_right_sensitiveness = " << left_right_sensitiveness << "\n";
	user_data << "left_right_speed = " << left_right_speed << "\n";
	user_data << "soft_drop_speed = " << soft_drop_speed << "\n";
	user_data.close();
}

void MyApp::create()
{
	App::create(false);

	auto user_data = parse_ini_file(L"user_data.ini");
	for (auto& e : user_data.get_section_entries(""))
	{
		if (e.key == "name")
			my_name = s2w(e.value);
	}
	auto key_info = find_enum("flame::KeyboardKey");
	for (auto& e : user_data.get_section_entries("key"))
	{
		for (auto i = 0; i < KEY_COUNT; i++)
		{
			if (key_names[i] == s2w(e.key))
				key_map[i] = (KeyboardKey)key_info->find_item(e.value.c_str())->get_value();
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

	atlas = graphics::ImageAtlas::create(graphics_device, (resource_path / L"assets/atlas/main.atlas").c_str());

	{
		sound_move_buf = sound::Buffer::create_from_file((resource_path / L"assets/move.wav").c_str());
		sound_move_src = sound::Source::create(sound_move_buf);
		sound_move_src->set_volume(sound_move_volumn);
	}
	{
		sound_soft_drop_buf = sound::Buffer::create_from_file((resource_path / L"assets/soft_drop.wav").c_str());
		sound_soft_drop_src = sound::Source::create(sound_soft_drop_buf);
		sound_soft_drop_src->set_volume(sound_soft_drop_volumn);
	}
	{
		sound_hard_drop_buf = sound::Buffer::create_from_file((resource_path / L"assets/hard_drop.wav").c_str());
		sound_hard_drop_src = sound::Source::create(sound_hard_drop_buf);
		sound_hard_drop_src->set_volume(sound_hard_drop_volumn);
	}
	{
		sound_clear_buf = sound::Buffer::create_from_file((resource_path / L"assets/clear.wav").c_str());
		sound_clear_src = sound::Source::create(sound_clear_buf);
		sound_clear_src->set_volume(sound_clear_volumn);
	}
	{
		sound_hold_buf = sound::Buffer::create_from_file((resource_path / L"assets/hold.wav").c_str());
		sound_hold_src = sound::Source::create(sound_hold_buf);
		sound_hold_src->set_volume(sound_hold_volumn);
	}
}

void MyApp::create_home_scene()
{
	ui.e_begin_layout(LayoutVertical, 8.f);
	ui.c_aligner(AlignMiddle, AlignMiddle);
	ui.push_style(FontSize, common(Vec1u(40)));
	ui.e_text(L"Tetris");
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.push_style(FontSize, common(Vec1u(20)));
	ui.e_button(L"Marathon", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.game_mode = GameSingleMarathon;
			g_app.create_game_scene();
			g_app.start_game();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"RTA", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.game_mode = GameSingleRTA;
			g_app.create_game_scene();
			g_app.start_game();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"Practice", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.game_mode = GameSinglePractice;
			g_app.create_game_scene();
			g_app.start_game();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"LAN", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_lan_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"Config", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_config_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.e_end_layout();
}

void MyApp::create_player_controls(int player_index)
{
	auto scale = (player_index == my_room_index || room_max_people == 2) ? 1.f : 0.5f;
	auto block_size = 24U * scale;

	auto pos = vec2(game_mode != GameVS ? 120.f : 0.f, 0.f);
	if (player_index != my_room_index)
	{
		switch (room_max_people)
		{
		case 2:
			pos = vec2(420.f, 0.f);
			break;
		case 7:
		{
			auto index = player_index;
			if (index > my_room_index)
				index--;
			pos = vec2(330.f + (index % 3) * 128.f, (index / 3) * 265.f);
		}
		break;
		}
	}

	auto& p = players[player_index];

	ui.next_element_pos = pos + vec2(80.f, 40.f) * scale;
	ui.e_begin_layout(LayoutHorizontal, 4.f);
	ui.push_style(FontSize, common(Vec1u(30 * scale)));
	p.c_name = ui.e_text([p]() {
		switch (g_app.game_mode)
		{
		case GameSingleMarathon:
			return L"Marathon";
		case GameSingleRTA:
			return L"RTA";
		case GameSinglePractice:
			return L"Practice";
		case GameVS:
			return p.name.c_str();
		}
	}())->get_component(cText);
	if (game_mode == GameVS && player_index == 0)
		p.c_name->color = cvec4(91, 82, 119, 255);
	ui.pop_style(FontSize);

	if (my_room_index == 0 && player_index != my_room_index)
	{
		p.e_kick = ui.e_button(Icon_TIMES, [](Capture& c) {
			auto index = c.data<int>();

			g_app.process_player_left(index);

			{
				nlohmann::json rep;
				rep["action"] = "player_left";
				rep["index"] = index;
				auto str = rep.dump();
				for (auto i = 1; i < g_app.players.size(); i++)
				{
					if (i != index)
					{
						auto& p = g_app.players[i];
						if (p.id && !p.disconnected)
							g_app.server->send(p.id, str.data(), str.size(), false);
					}
				}
			}
		}, Capture().set_data(&player_index));
	}
	ui.e_end_layout();

	ui.e_empty();
	ui.next_element_pos = pos + vec2(85.f, 80.f) * scale;
	ui.next_element_size = vec2(block_size * board_width, block_size * (board_height - 3.8f));
	{
		auto ce = ui.c_element();
		ce->frame_thickness = 6.f * scale;
		ce->color = cvec4(30, 30, 30, 255);
		ce->frame_color = cvec4(255);
		ce->clip_flags = ClipChildren;;
	}

	ui.parents.push(ui.current_entity);
	ui.e_empty();
	ui.next_element_pos = vec2(0.f, -block_size * 3.8f);
	ui.next_element_size = vec2(block_size * board_width, block_size * board_height);
	ui.c_element();
	p.c_main = cTileMap::create();
	p.c_main->cell_size_ = vec2(block_size);
	p.c_main->set_size(uvec2(board_width, board_height));
	p.c_main->clear_cells(TileGrid);
	set_board_tiles(p.c_main);
	ui.current_entity->add_component(p.c_main);
	ui.parents.pop();

	if (player_index == my_room_index)
	{
		block_size = 16U * scale;

		ui.next_element_pos = pos + vec2(22.f, 80.f);
		ui.e_text(L"Hold");

		ui.next_element_pos = pos + vec2(8.f, 100.f);
		ui.next_element_size = vec2(block_size * 4 + 8.f);
		ui.next_element_padding = 4.f;
		ui.next_element_color = cvec4(30, 30, 30, 255);
		ui.e_element();
		{
			p.c_hold = cTileMap::create();
			p.c_hold->cell_size_ = vec2(block_size);
			p.c_hold->set_size(uvec2(4, 3));
			set_board_tiles(p.c_hold);
			ui.current_entity->add_component(p.c_hold);
		}

		ui.next_element_pos = pos + vec2(350.f, 80.f);
		ui.e_text(L"Next");

		ui.e_empty();
		ui.next_element_pos = pos + vec2(330.f, 100.f);
		ui.next_element_size = vec2(block_size * 4 + 8.f, (block_size * 3.f + 4.f) * size(p.c_next) + 8.f - 45.f);
		{
			auto ce = ui.c_element();
			ce->color = cvec4(30, 30, 30, 255);
		}
		auto create_next_board = [&](int i, int base, float y_off, float block_size) {
			ui.next_element_pos = pos + vec2(330.f, 100.f + y_off + (block_size * 3.f + 4.f) * (i - base));
			ui.next_element_size = vec2(block_size * 4 + 8.f);
			ui.next_element_padding = 4.f;
			ui.e_element();
			{
				p.c_next[i] = cTileMap::create();
				p.c_next[i]->cell_size_ = vec2(block_size);
				p.c_next[i]->set_size(uvec2(4));
				set_board_tiles(p.c_next[i]);
				ui.current_entity->add_component(p.c_next[i]);
			}
		};
		for (auto i = 0; i < 1; i++)
			create_next_board(i, 0, 0.f, 16.f);
		for (auto i = 1; i < 3; i++)
			create_next_board(i, 1, 16.f * 3.f + 4.f, 14.f);
		for (auto i = 3; i < size(p.c_next); i++)
			create_next_board(i, 3, 16.f * 3.f + 4.f + (14.f * 3.f + 4.f) * 2, 12.f);

		ui.next_element_pos = pos + vec2(180.f, 250.f);
		ui.push_style(FontSize, common(Vec1u(80)));
		p.e_count_down = ui.e_text(L"");
		p.e_count_down->set_visible(false);
		ui.pop_style(FontSize);

		if (game_mode == GameVS)
		{
			ui.next_element_pos = pos + vec2(54.f, 546.f);
			p.e_garbage = ui.e_element();
		}
	}

	if (game_mode == GameVS)
	{
		ui.push_style(FontSize, common(Vec1u(60 * scale)));
		ui.next_element_pos = pos + vec2(150.f, 200.f) * scale;
		p.c_ready = ui.e_text(L"Ready")->get_component(cText);
		p.c_ready->entity->set_visible(false);
		ui.next_element_pos = pos + vec2(160.f, 150.f) * scale;
		p.c_rank = ui.e_text(L"Ready")->get_component(cText);
		p.c_rank->entity->set_visible(false);
		ui.pop_style(FontSize);
	}
}

void MyApp::process_player_entered(int index)
{
	looper().add_event([](Capture& c) {
		auto index = c.data<int>();
		auto& p = g_app.players[index];
		ui.parents.push(main_window->root);
		p.e = ui.e_element();
		ui.parents.push(p.e);
		g_app.create_player_controls(index);
		ui.parents.pop();
		ui.parents.pop();
	}, Capture().set_data(&index));
}

void MyApp::process_player_disconnected(int index)
{
	looper().add_event([](Capture& c) {
		auto index = c.data<int>();
		auto& p = g_app.players[index];
		p.disconnected = true;
		p.c_name->set_text((p.name + L" " + Icon_BOLT).c_str());
	}, Capture().set_data(&index));
}

void MyApp::process_player_left(int index)
{
	looper().add_event([](Capture& c) {
		auto index = c.data<int>();
		auto& p = g_app.players[index];
		p.reset();
		main_window->root->remove_child(p.e);
	}, Capture().set_data(&index));
}

void MyApp::process_player_ready(int index)
{
	looper().add_event([](Capture& c) {
		auto& p = g_app.players[c.data<int>()];
		p.ready = true;
		p.c_ready->entity->set_visible(true);
	}, Capture().set_data(&index));
}

void MyApp::process_game_start()
{
	looper().add_event([](Capture&) {
		g_app.room_gaming = true;
		g_app.start_game();
	}, Capture());
}

void MyApp::process_report_board(int index, std::string_view d)
{
	struct Capturing
	{
		cTileMap* b;
		char d[1024];
	}capture;
	capture.b = g_app.players[index].c_main;
	memcpy(capture.d, d.data(), d.size());
	looper().add_event([](Capture& c) {
		auto& capture = c.data<Capturing>();
		for (auto y = 0; y < board_height; y++)
		{
			for (auto x = 0; x < board_width; x++)
			{
				auto id = capture.d[y * board_width + x] - '0';
				capture.b->set_cell(uvec2(x, y), id, id == TileGrid ? cvec4(255) : mino_col_decay);
			}
		}
	}, Capture().set_data(&capture));
}

void MyApp::process_attack(int index, int value)
{
	looper().add_event([](Capture& c) {
		auto n = c.data<int>();
		Garbage g;
		g.time = 60;
		g.lines = n;
		g_app.garbages.push_back(g);
		g_app.need_update_garbages_tip = true;
	}, Capture().set_data(&value));
}

void MyApp::process_dead(int index, int rank)
{
	struct Capturing
	{
		int index;
		int rank;
	}capture;
	capture.index = index;
	capture.rank = rank;
	looper().add_event([](Capture& c) {
		auto& capture = c.data<Capturing>();
		auto& p = g_app.players[capture.index];
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
		p.c_rank->entity->set_visible(true);
	}, Capture().set_data(&capture));
}

void MyApp::process_gameover()
{
	looper().add_event([](Capture&) {
		g_app.room_gaming = false;
		g_app.gaming = false;
		g_app.e_start_or_ready->set_visible(true);
	}, Capture());
}

void MyApp::join_room(const char* ip)
{
	g_app.client = Client::create(SocketTcp, ip, 2434,
	[](Capture&, const char* msg, uint size) {
		auto req = nlohmann::json::parse(std::string(msg, size));
		auto action = req["action"].get<std::string>();
		if (action == "report_room")
		{
			g_app.room_name = s2w(req["room_name"].get<std::string>());
			g_app.room_max_people = req["max_people"].get<int>();
			g_app.players.resize(g_app.room_max_people);
			for (auto& p : g_app.players)
				p.reset();
			g_app.my_room_index = req["index"].get<int>();
			auto& me = g_app.players[g_app.my_room_index];
			me.id = (void*)0xffff;
			me.name = g_app.my_name;
			looper().add_event([](Capture&) {
				main_window->root->remove_children(1, -1);
				g_app.game_mode = GameVS;
				g_app.create_game_scene();
			}, Capture());
		}
		else if (action == "player_entered")
		{
			auto index = req["index"].get<int>();
			auto& p = g_app.players[index];
			p.id = (void*)0xffff;
			p.name = s2w(req["name"].get<std::string>());

			g_app.process_player_entered(index);
		}
		else if (action == "player_disconnected")
			g_app.process_player_disconnected(req["index"].get<int>());
		else if (action == "player_left")
			g_app.process_player_left(req["index"].get<int>());
		else if (action == "player_ready")
			g_app.process_player_ready(req["index"].get<int>());
		else if (action == "game_start")
			g_app.process_game_start();
		else if (action == "report_board")
			g_app.process_report_board(req["index"].get<int>(), req["board"].get<std::string>());
		else if (action == "report_dead")
			g_app.process_dead(req["index"].get<int>(), req["rank"].get<int>());
		else if (action == "report_gameover")
			g_app.process_gameover();
		else if (action == "attack")
		{
			auto index = req["index"].get<int>();
			auto value = req["value"].get<int>();
			g_app.process_attack(index, value);
		}
	},
	[](Capture&) {
		looper().add_event([](Capture&) {
			ui.e_message_dialog(L"Host Has Disconnected")->event_listeners.add([](Capture&, EntityEvent e, void*) {
				if (e == EntityDestroyed)
				{
					looper().add_event([](Capture&) {
						g_app.quit_game();
					}, Capture());
				}
				return true;
			}, Capture());
		}, Capture());
	}, Capture());
	if (g_app.client)
	{
		nlohmann::json req;
		req["action"] = "join_room";
		req["name"] = w2s(g_app.my_name);
		auto str = req.dump();
		g_app.client->send(str.data(), str.size());
	}
	else
		ui.e_message_dialog(L"Join Room Failed");
}

void MyApp::people_dead(int index)
{
	auto& p = players[index];
	p.dead = true;

	auto total_people = 0;
	auto dead_people = 0;
	auto last_people = 0;
	for (auto i = 0; i < g_app.players.size(); i++)
	{
		auto& p = g_app.players[i];
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
		for (auto i = 1; i < g_app.players.size(); i++)
		{
			auto& p = g_app.players[i];
			if (p.id && !p.disconnected)
				g_app.server->send(p.id, str.data(), str.size(), false);
		}
	}

	process_dead(index, rank);

	if (total_people - dead_people == 1)
	{
		g_app.process_dead(last_people, 1);

		{
			nlohmann::json rep;
			rep["action"] = "report_dead";
			rep["index"] = last_people;
			rep["rank"] = 1;
			auto str = rep.dump();
			for (auto i = 1; i < g_app.players.size(); i++)
			{
				auto& p = g_app.players[i];
				if (p.id && !p.disconnected)
					g_app.server->send(p.id, str.data(), str.size(), false);
			}
		}

		for (auto i = 1; i < g_app.players.size(); i++)
		{
			auto& p = g_app.players[i];
			if (p.id)
				p.e_kick->set_visible(true);
		}
		g_app.process_gameover();

		{
			nlohmann::json rep;
			rep["action"] = "report_gameover";
			auto str = rep.dump();
			for (auto i = 1; i < g_app.players.size(); i++)
			{
				auto& p = g_app.players[i];
				if (p.id && !p.disconnected)
					g_app.server->send(p.id, str.data(), str.size(), false);
			}
		}
	}
}

void MyApp::create_lan_scene()
{
	ui.parents.push(main_window->root);
	ui.next_element_size = vec2(500.f, 0.f);
	ui.next_element_padding = 8.f;
	ui.e_begin_layout(LayoutVertical, 8.f, false, false);
	ui.c_aligner(AlignMiddle, AlignMinMax);
	ui.push_style(FontSize, common(Vec1u(20)));
	ui.e_begin_layout(LayoutHorizontal, 8.f);
	ui.c_aligner(AlignMiddle, 0);
	ui.e_text(L"Your Name");
	ui.e_edit(300.f, g_app.my_name.c_str())->get_component(cText)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
		if (hash == FLAME_CHASH("text"))
			g_app.my_name = c.current<cText>()->text.str();
		return true;
	}, Capture());
	ui.e_end_layout();
	ui.next_element_padding = 4.f;
	ui.next_element_frame_thickness = 2.f;
	ui.e_begin_scrollbar(ScrollbarVertical, true);
	auto e_room_list = ui.e_begin_list(true);
	ui.e_end_list();
	ui.e_end_scrollbar();
	ui.e_begin_layout(LayoutHorizontal, 8.f)->get_component(cLayout)->fence = -1;
	ui.c_aligner(AlignMinMax, 0);
	ui.e_button(Icon_REFRESH, [](Capture& c) {
		auto e_room_list = c.thiz<Entity>();
		looper().add_event([](Capture& c) {
			auto e_room_list = c.thiz<Entity>();
			e_room_list->remove_children(0, -1);
			nlohmann::json req;
			req["action"] = "get_room";
			auto str = req.dump();
			board_cast(2434, str.data(), str.size(), 1, [](Capture& c, const char* ip, const char* msg, uint size) {
				auto e_room_list = c.thiz<Entity>();
				auto rep = nlohmann::json::parse(std::string(msg, size));
				auto name = s2w(rep["name"].get<std::string>());
				auto host = s2w(rep["host"].get<std::string>());

				ui.parents.push(e_room_list);
				ui.e_list_item((L"Name:" + name + L" Host:" + host).c_str());
				ui.c_data_keeper()->set_string_item(FLAME_CHASH("ip"), ip);
				ui.parents.pop();
			}, Capture().set_thiz(e_room_list));
		}, Capture().set_thiz(e_room_list));
	}, Capture().set_thiz(e_room_list))->get_component(cReceiver)->send_mouse_event(KeyStateDown | KeyStateUp, Mouse_Null, ivec2(0));
	ui.e_button(L"Create Room", [](Capture&) {
		if (g_app.my_name.empty())
			ui.e_message_dialog(L"Your Name Cannot Not Be Empty");
		else
		{
			auto e_layer = ui.e_begin_dialog()->parent;
			ui.e_text(L"Room Name");
			ui.e_edit(100.f)->get_component(cText)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("text"))
					g_app.room_name = c.current<cText>()->text.str();
				return true;
			}, Capture());
			ui.e_text(L"Max People");
			ui.e_begin_combobox()->get_component(cCombobox)->data_changed_listeners.add([](Capture& c, uint hash, void*) {
				if (hash == FLAME_CHASH("index"))
				{
					auto index = c.current<cCombobox>()->index;
					switch (index)
					{
					case 0:
						g_app.room_max_people = 2;
						break;
					case 1:
						g_app.room_max_people = 7;
						break;
					}
				}
				return true;
			}, Capture());
			ui.e_combobox_item(L"2");
			ui.e_combobox_item(L"7");
			ui.e_end_combobox(0);
			g_app.room_max_people = 2;
			ui.e_begin_layout(LayoutHorizontal, 4.f);
			ui.c_aligner(AlignMiddle, 0);
			ui.e_button(L"OK", [](Capture& c) {
				remove_layer(c.thiz<Entity>());

				if (!g_app.room_name.empty())
				{
					g_app.players.resize(g_app.room_max_people);
					for (auto& p : g_app.players)
						p.reset();
					g_app.my_room_index = 0;
					{
						auto& me = g_app.players[0];
						me.id = (void*)0xffff;
						me.name = g_app.my_name;
					}
					g_app.server = Server::create(SocketTcp, 2434,
					[](Capture&, void* id, const char* msg, uint size) {
						auto req = nlohmann::json::parse(std::string(msg, size));
						if (req["action"] == "get_room")
						{
							nlohmann::json rep;
							rep["name"] = w2s(g_app.room_name);
							rep["host"] = w2s(g_app.my_name);
							auto str = rep.dump();
							g_app.server->send(id, str.data(), str.size(), true);
						}
					},
					[](Capture&, void* id) {
						if (!g_app.room_gaming)
						{
							for (auto i = 0; i < g_app.players.size(); i++)
							{
								auto& p = g_app.players[i];
								if (!p.id)
								{
									{
										nlohmann::json rep;
										rep["action"] = "report_room";
										rep["room_name"] = w2s(g_app.room_name);
										rep["max_people"] = g_app.room_max_people;
										rep["index"] = i;
										auto str = rep.dump();
										g_app.server->send(id, str.data(), str.size(), false);
									}
									{
										nlohmann::json rep;
										rep["action"] = "player_entered";
										rep["index"] = g_app.my_room_index;
										rep["name"] = w2s(g_app.my_name);
										auto str = rep.dump();
										g_app.server->send(id, str.data(), str.size(), false);
									}

									p.id = id;
									g_app.server->set_client(id,
										[](Capture& c, const char* msg, uint size) {
										auto index = c.data<int>();
										auto& p = g_app.players[index];
										auto req = nlohmann::json::parse(std::string(msg, size));
										auto action = req["action"].get<std::string>();
										if (action == "join_room")
										{
											p.name = s2w(req["name"].get<std::string>());

											g_app.process_player_entered(index);

											{
												nlohmann::json rep;
												rep["action"] = "player_entered";
												rep["index"] = index;
												rep["name"] = w2s(p.name);
												auto str = rep.dump();
												for (auto i = 1; i < g_app.players.size(); i++)
												{
													if (i != index)
													{
														auto& p = g_app.players[i];
														if (p.id && !p.disconnected)
															g_app.server->send(p.id, str.data(), str.size(), false);
													}
												}
											}
										}
										else if (action == "ready")
										{
											g_app.process_player_ready(index);

											{
												nlohmann::json rep;
												rep["action"] = "player_ready";
												rep["index"] = index;
												auto str = rep.dump();
												for (auto i = 1; i < g_app.players.size(); i++)
												{
													if (i != index)
													{
														auto& p = g_app.players[i];
														if (p.id && !p.disconnected)
															g_app.server->send(p.id, str.data(), str.size(), false);
													}
												}
											}
										}
										else if (action == "report_board")
										{
											auto d = req["board"].get<std::string>();
											g_app.process_report_board(req["index"].get<int>(), d);

											{
												nlohmann::json rep;
												rep["action"] = "report_board";
												rep["index"] = index;
												rep["board"] = d;
												auto str = rep.dump();
												for (auto i = 1; i < g_app.players.size(); i++)
												{
													if (i != index)
													{
														auto& p = g_app.players[i];
														if (p.id && !p.disconnected)
															g_app.server->send(p.id, str.data(), str.size(), false);
													}
												}
											}
										}
										else if (action == "report_dead")
											g_app.people_dead(index);
										else if (action == "attack")
										{
											auto target = req["target"].get<int>();
											auto value = req["value"].get<int>();
											if (target == g_app.my_room_index)
												g_app.process_attack(index, value);
											else
											{
												nlohmann::json rep;
												rep["action"] = "attack";
												rep["index"] = index;
												rep["value"] = value;
												auto str = rep.dump();
												auto& p = g_app.players[target];
												if (p.id && !p.disconnected)
													g_app.server->send(p.id, str.data(), str.size(), false);
											}
										}
									},
										[](Capture& c) {
										auto index = c.data<int>();

										g_app.process_player_disconnected(index);

										{
											nlohmann::json rep;
											rep["action"] = "player_disconnected";
											rep["index"] = index;
											auto str = rep.dump();
											for (auto i = 1; i < g_app.players.size(); i++)
											{
												if (i != index)
												{
													auto& p = g_app.players[i];
													if (p.id && !p.disconnected)
														g_app.server->send(p.id, str.data(), str.size(), false);
												}
											}
										}

										if (g_app.room_gaming)
											g_app.people_dead(index);
									}, Capture().set_data(&i));

									break;
								}
							}
						}
					}, Capture());
					g_app.room_gaming = false;
					looper().add_event([](Capture&) {
						main_window->root->remove_children(1, -1);
						g_app.game_mode = GameVS;
						g_app.create_game_scene();
					}, Capture());
				}
			}, Capture().set_thiz(e_layer));
			ui.e_button(L"Cancel", [](Capture& c) {
				remove_layer(c.thiz<Entity>());
			}, Capture().set_thiz(e_layer));
			ui.e_end_layout();
			ui.e_end_dialog();
		}
	}, Capture());
	ui.e_button(L"Join Room", [](Capture& c) {
		auto e_room_list = c.thiz<Entity>();
		auto selected = e_room_list->get_component(cList)->selected;
		if (selected)
		{
			if (g_app.my_name.empty())
				ui.e_message_dialog(L"Your Name Cannot Not Be Empty");
			else
				g_app.join_room(selected->get_component(cDataKeeper)->get_string_item(FLAME_CHASH("ip")));
		}
		else
			ui.e_message_dialog(L"You Need To Select A Room");
	}, Capture().set_thiz(e_room_list));
	ui.e_button(L"Direct Connect", [](Capture&) {
		if (g_app.my_name.empty())
			ui.e_message_dialog(L"Your Name Cannot Not Be Empty");
		else
		{
			ui.e_input_dialog(L"IP", [](Capture&, bool ok, const wchar_t* text) {
				if (ok)
					g_app.join_room(w2s(text).c_str());
			}, Capture());
		}
	}, Capture());
	ui.e_button(L"Back", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_home_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMax, AlignMin);
	ui.e_end_layout();
	ui.pop_style(FontSize);
	ui.e_end_layout();
	ui.parents.pop();
}

void MyApp::create_config_scene()
{
	ui.parents.push(main_window->root);
	ui.e_begin_layout(LayoutVertical, 8.f);
	ui.c_aligner(AlignMiddle, AlignMiddle);
	ui.push_style(FontSize, common(Vec1u(20)));
	ui.e_button(L"KeyboardKey", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_key_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"Sound", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_sound_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"Sensitiveness", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_sensitiveness_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.e_button(L"Back", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_home_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.e_end_layout();
	ui.parents.pop();
}

void MyApp::create_key_scene()
{
	ui.parents.push(main_window->root);
	ui.e_begin_layout(LayoutVertical, 8.f);
	ui.c_aligner(AlignMiddle, AlignMiddle);
	ui.push_style(FontSize, common(Vec1u(20)));
	auto key_info = find_enum(FLAME_CHASH("flame::KeyboardKey"));
	for (auto i = 0; i < KEY_COUNT; i++)
	{
		ui.e_begin_layout(LayoutHorizontal, 4.f);
		ui.e_text(key_names[i]);
		struct Capturing
		{
			cText* t;
			int i;
		}capture;
		auto e_edit = ui.e_edit(200.f, s2w(key_info->find_item(key_map[i])->name.str()).c_str());
		capture.t = e_edit->get_component(cText);
		capture.i = i;
		e_edit->get_component(cReceiver)->key_listeners.add([](Capture& c, KeyStateFlags action, int value) {
			if (action == KeyStateDown)
			{
				auto& capture = c.data<Capturing>();
				key_map[capture.i] = (KeyboardKey)value;
				auto key_info = find_enum(FLAME_CHASH("flame::KeyboardKey"));
				capture.t->set_text(s2w(key_info->find_item((KeyboardKey)value)->name.str()).c_str());
			}
			return false;
		}, Capture().set_data(&capture), 0);
		ui.c_aligner(AlignMinMax | AlignGreedy, 0);
		ui.e_end_layout();
	}
	ui.e_button(L"Back", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_config_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.e_end_layout();
	ui.parents.pop();
}

void MyApp::create_sound_scene()
{
	ui.parents.push(main_window->root);
	ui.e_begin_layout(LayoutVertical, 8.f);
	ui.c_aligner(AlignMiddle, AlignMiddle);
	ui.push_style(FontSize, common(Vec1u(20)));
	struct Capturing
	{
		cText* t;
		int v;
	}capture;
	ui.e_begin_layout(LayoutHorizontal, 4.f);
	capture.t = ui.e_text(wfmt(L"FX %d", fx_volumn).c_str())->get_component(cText);
	auto change_fx_volumn = [](Capture& c) {
		auto& capture = c.data<Capturing>();
		auto v = fx_volumn + capture.v;
		if (v >= 0 && v <= 10)
		{
			fx_volumn = v;
			capture.t->set_text(wfmt(L"FX %d", v).c_str());

			auto f = v / 10.f;
			g_app.sound_move_src->set_volume(f * sound_move_volumn);
			g_app.sound_soft_drop_src->set_volume(f * sound_soft_drop_volumn);
			g_app.sound_hard_drop_src->set_volume(f * sound_hard_drop_volumn);
			g_app.sound_clear_src->set_volume(f * sound_clear_volumn);
			g_app.sound_hold_src->set_volume(f * sound_hold_volumn);
		}
	};
	capture.v = -1;
	ui.e_button(L"-", change_fx_volumn, Capture().set_data(&capture));
	capture.v = 1;
	ui.e_button(L"+", change_fx_volumn, Capture().set_data(&capture));
	ui.e_end_layout();
	ui.e_button(L"Back", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_config_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.e_end_layout();
	ui.parents.pop();
}

void MyApp::create_sensitiveness_scene()
{
	ui.parents.push(main_window->root);
	ui.e_begin_layout(LayoutVertical, 8.f);
	ui.c_aligner(AlignMiddle, AlignMiddle);
	ui.push_style(FontSize, common(Vec1u(20)));
	ui.e_text(L"Small Number Means More Sensitivity Or Faster");
	struct Capturing
	{
		cText* t;
		int v;
	}capture;
	ui.e_begin_layout(LayoutHorizontal, 4.f);
	capture.t = ui.e_text(wfmt(L"Left Right Sensitiveness %d",
		left_right_sensitiveness).c_str())->get_component(cText);
	auto change_lr_sens = [](Capture& c) {
		auto& capture = c.data<Capturing>();
		auto v = left_right_sensitiveness + capture.v;
		if (v >= 5 && v <= 30)
		{
			left_right_sensitiveness = v;
			capture.t->set_text(wfmt(L"Left Right Sensitiveness %d", v).c_str());
		}
	};
	capture.v = -1;
	ui.e_button(L"-", change_lr_sens, Capture().set_data(&capture));
	capture.v = 1;
	ui.e_button(L"+", change_lr_sens, Capture().set_data(&capture));
	ui.e_end_layout();
	ui.e_begin_layout(LayoutHorizontal, 4.f);
	capture.t = ui.e_text(wfmt(L"Left Right Speed %d",
		left_right_speed).c_str())->get_component(cText);
	auto change_lr_sp = [](Capture& c) {
		auto& capture = c.data<Capturing>();
		auto v = left_right_speed + capture.v;
		if (v >= 1 && v <= 10)
		{
			left_right_speed = v;
			capture.t->set_text(wfmt(L"Left Right Speed %d", v).c_str());
		}
	};
	capture.v = -1;
	ui.e_button(L"-", change_lr_sp, Capture().set_data(&capture));
	capture.v = 1;
	ui.e_button(L"+", change_lr_sp, Capture().set_data(&capture));
	ui.e_end_layout();
	ui.e_begin_layout(LayoutHorizontal, 4.f);
	capture.t = ui.e_text(wfmt(L"Soft Drop Speed %d",
		soft_drop_speed).c_str())->get_component(cText);
	auto change_sd_sp = [](Capture& c) {
		auto& capture = c.data<Capturing>();
		auto v = soft_drop_speed + capture.v;
		if (v >= 1 && v <= 10)
		{
			soft_drop_speed = v;
			capture.t->set_text(wfmt(L"Soft Drop Speed %d", v).c_str());
		}
	};
	capture.v = -1;
	ui.e_button(L"-", change_sd_sp, Capture().set_data(&capture));
	capture.v = 1;
	ui.e_button(L"+", change_sd_sp, Capture().set_data(&capture));
	ui.e_end_layout();
	ui.e_button(L"Back", [](Capture&) {
		looper().add_event([](Capture&) {
			main_window->root->remove_children(1, -1);
			g_app.create_config_scene();
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMiddle, 0);
	ui.pop_style(FontSize);
	ui.e_end_layout();
	ui.parents.pop();
}

void MyApp::set_board_tiles(cTileMap* m)
{
	m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile("grid.png"));
	for (auto i = 1; i <= 7; i++)
		m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile((std::to_string(i) + ".png").c_str()));
	m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile("gray.png"));
	m->add_tile((atlas->canvas_slot_ << 16) + atlas->find_tile("ghost.png"));
}

void MyApp::create_game_scene()
{
	ui.parents.push(main_window->root);
	ui.push_style(FontSize, common(Vec1u(20)));

	ui.e_empty();
	{
		auto ev = looper().add_event([](Capture& c) {
			c._current = nullptr;
		}, Capture());
		ui.current_entity->event_listeners.add([](Capture& c, EntityEvent e, void*) {
			if (e == EntityRemoved)
				looper().remove_event(c.data<void*>());
			return true;
		}, Capture().set_data(&ev));
	}

	if (game_mode == GameVS)
		ui.e_text(wfmt(L"Room: %s", room_name.c_str()).c_str());

	create_player_controls(my_room_index);

	if (game_mode != GameVS)
	{
		ui.next_element_pos = vec2(535.f, 150.f);
		ui.e_text(L"TIME")->get_component(cText)->color = cvec4(40, 80, 200, 255);

		ui.next_element_pos = vec2(535.f, 210.f);
		ui.e_text(L"LEVEL")->get_component(cText)->color = cvec4(40, 80, 200, 255);

		ui.next_element_pos = vec2(535.f, 270.f);
		ui.e_text(game_mode == GameSingleRTA ? L"LEFT" : L"LINES")->get_component(cText)->color = cvec4(40, 80, 200, 255);

		ui.next_element_pos = vec2(535.f, 330.f);
		ui.e_text(L"SCORE")->get_component(cText)->color = cvec4(40, 80, 200, 255);

		ui.push_style(FontSize, common(Vec1u(40)));
		ui.next_element_pos = vec2(535.f, 170.f);
		c_text_time = ui.e_text(L"")->get_component(cText);
		ui.next_element_pos = vec2(535.f, 230.f);
		c_text_level = ui.e_text(L"")->get_component(cText);
		ui.next_element_pos = vec2(535.f, 290.f);
		c_text_lines = ui.e_text(L"")->get_component(cText);
		ui.next_element_pos = vec2(535.f, 350.f);
		c_text_score = ui.e_text(L"")->get_component(cText);
		ui.pop_style(FontSize);
	}

	ui.push_style(FontSize, common(Vec1u(28)));
	ui.next_element_pos = vec2(8.f, 230.f);
	{
		auto e = ui.e_text(L"");
		e->set_visible(false);
		c_text_special = e->get_component(cText);
	}
	c_text_special->color = cvec4(200, 80, 40, 255);
	ui.pop_style(FontSize);

	if (game_mode == GameVS)
	{
		ui.next_element_pos = vec2(4.f, 500.f);
		if (my_room_index == 0)
		{
			e_start_or_ready = ui.e_button(L"Start", [](Capture&) {
				if ([]() {
					auto n = 0;
						for (auto i = 1; i < g_app.players.size(); i++)
						{
							auto& p = g_app.players[i];
								if (!p.id)
									continue;
								if (!p.ready)
									return false;
								n++;
						}
					return n != 0;
				}())
				{
					for (auto i = 1; i < g_app.players.size(); i++)
					{
						auto& p = g_app.players[i];
						if (p.id && !p.disconnected)
							p.e_kick->set_visible(false);
					}
					g_app.process_game_start();

					{
						nlohmann::json req;
						req["action"] = "game_start";
						auto str = req.dump();
						for (auto i = 1; i < g_app.players.size(); i++)
						{
							auto& p = g_app.players[i];
							if (p.id && !p.disconnected)
								g_app.server->send(p.id, str.data(), str.size(), false);
						}
					}
				}
			}, Capture());
		}
		else
		{
			e_start_or_ready = ui.e_button(L"Ready", [](Capture&) {
				auto& me = g_app.players[g_app.my_room_index];
				if (!g_app.room_gaming && !me.ready)
				{
					me.ready = true;
					nlohmann::json req;
					req["action"] = "ready";
					auto str = req.dump();
					g_app.client->send(str.data(), str.size());

					g_app.process_player_ready(g_app.my_room_index);
				}
			}, Capture());
		}
	}

	ui.e_button(Icon_TIMES, [](Capture&) {
		ui.e_confirm_dialog(L"Quit?", [](Capture&, bool yes) {
			if (yes)
			{
				looper().add_event([](Capture&) {
					g_app.quit_game();
				}, Capture());
			}
		}, Capture());
	}, Capture());
	ui.c_aligner(AlignMax, AlignMin);

	ui.pop_style(FontSize);
	ui.parents.pop();
}

void MyApp::begin_count_down()
{
	auto e_count_down = players[my_room_index].e_count_down;
	e_count_down->set_visible(true);
	e_count_down->get_component(cText)->set_text(L"3");
	auto t = 3;
	looper().remove_events(FLAME_CHASH("count_down"));
	looper().add_event([](Capture& c) {
		auto& t = c.data<int>();
		t--;

		auto e = c.thiz<Entity>();
		if (t == 0)
		{
			e->set_visible(false);
			g_app.gaming = true;
		}
		else
		{
			c._current = nullptr;
			e->get_component(cText)->set_text(std::to_wstring(t).c_str());
		}
	}, Capture().set_thiz(e_count_down).set_data(&t), 1.f, FLAME_CHASH("count_down"));
}

void MyApp::update_status()
{
	if (game_mode != GameVS)
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

void MyApp::start_game()
{
	for (auto i = 0; i < players.size(); i++)
	{
		auto& p = g_app.players[i];
		if (p.id)
		{
			p.ready = false;
			p.dead = false;
			p.c_main->clear_cells(TileGrid);
			if (i == g_app.my_room_index)
			{
				p.c_hold->clear_cells(-1);
				for (auto j = 0; j < size(p.c_next); j++)
					p.c_next[j]->clear_cells(-1);
			}
			if (game_mode == GameVS)
			{
				p.c_ready->entity->set_visible(false);
				p.c_rank->entity->set_visible(false);
			}
		}
	}
	if (game_mode == GameVS)
		e_start_or_ready->set_visible(false);

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
	need_update_garbages_tip = true;
	mino_pos = ivec2(0, -1);
	mino_type = MinoTypeCount;
	mino_hold = MinoTypeCount;
	mino_just_hold = false;
	mino_pack_idx = uvec2(0, 0);
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
	if (game_mode == GameVS)
		players[my_room_index].e_garbage->remove_children(0, -1);

	gaming = false;
	begin_count_down();
}

void MyApp::shuffle_pack(uint idx)
{
	auto& curr_pack = mino_packs[idx];
	for (auto i = 0; i < MinoTypeCount; i++)
		curr_pack[i] = (MinoType)i;
	for (auto i = 0; i < MinoTypeCount; i++)
		std::swap(curr_pack[i], curr_pack[rand() % MinoTypeCount]);
}

void MyApp::draw_mino(cTileMap* board, int idx, const ivec2& pos, uint offset_y, ivec2* coords, const cvec4& col)
{
	board->set_cell(uvec2(pos) + uvec2(0, offset_y), idx, col);
	board->set_cell(uvec2(pos + coords[0] + uvec2(0, offset_y)), idx, col);
	board->set_cell(uvec2(pos + coords[1] + uvec2(0, offset_y)), idx, col);
	board->set_cell(uvec2(pos + coords[2] + uvec2(0, offset_y)), idx, col);
}

bool MyApp::check_board(cTileMap* board, const ivec2& p)
{
	return
		board->cell(mino_pos + p) == TileGrid &&
		board->cell(mino_pos + p + mino_coords[0]) == TileGrid &&
		board->cell(mino_pos + p + mino_coords[1]) == TileGrid &&
		board->cell(mino_pos + p + mino_coords[2]) == TileGrid;
}

bool MyApp::check_board(cTileMap* board, ivec2* in, const ivec2& p)
{
	return
		board->cell(p) == TileGrid &&
		board->cell(in[0] + p) == TileGrid &&
		board->cell(in[1] + p) == TileGrid &&
		board->cell(in[2] + p) == TileGrid;
}

bool MyApp::line_empty(cTileMap* board, uint l)
{
	for (auto x = 0; x < board_width; x++)
	{
		if (board->cell(ivec2(x, l)) != TileGrid)
			return false;
	}
	return true;
}

bool MyApp::line_full(cTileMap* board, uint l)
{
	for (auto x = 0; x < board_width; x++)
	{
		if (board->cell(ivec2(x, l)) == TileGrid)
			return false;
	}
	return true;
}

uint MyApp::get_rotation_idx(bool clockwise)
{
	if (clockwise)
		return mino_rotation == 3 ? 0 : mino_rotation + 1;
	return mino_rotation == 0 ? 3 : mino_rotation - 1;
}

bool MyApp::super_rotation(cTileMap* board, bool clockwise, ivec2* out_coord, ivec2* offset)
{
	Mat2x2i mats[] = {
		Mat2x2i(ivec2(0, -1), ivec2(1, 0)),
		Mat2x2i(ivec2(0, 1), ivec2(-1, 0))
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

void MyApp::quit_game()
{
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

	looper().add_event([](Capture&) {
		main_window->root->remove_children(1, -1);
		g_app.create_home_scene();
	}, Capture());
}

void MyApp::do_game_logic()
{
	auto& key_states = main_window->s_dispatcher->key_states;

	if (game_mode != GameVS)
	{
		if (key_states[key_map[KEY_PAUSE]] == (KeyStateDown | KeyStateJust))
		{
			auto layer = main_window->root->last_child(FLAME_CHASH("layer_paused"));
			if (!layer)
			{
				gaming = false;
				players[my_room_index].e_count_down->set_visible(false);

				auto layer = ui.e_begin_dialog()->parent;
				layer->name = "layer_paused";
				ui.e_text(L"Paused");
				ui.c_aligner(AlignMiddle, 0);
				ui.e_button(L"Resume", [](Capture& c) {
					remove_layer(c.thiz<Entity>());
					g_app.begin_count_down();
				}, Capture().set_thiz(layer));
				ui.c_aligner(AlignMiddle, 0);
				ui.e_button(L"Restart", [](Capture& c) {
					remove_layer(c.thiz<Entity>());
					g_app.play_time = 0.f;
					g_app.start_game();
				}, Capture().set_thiz(layer));
				ui.c_aligner(AlignMiddle, 0);
				ui.e_button(L"Quit", [](Capture& c) {
					remove_layer(c.thiz<Entity>());
					g_app.quit_game();
				}, Capture().set_thiz(layer));
				ui.c_aligner(AlignMiddle, 0);
				ui.e_end_dialog();
			}
			else
			{
				remove_layer(layer);
				begin_count_down();
			}
		}
	}

	if (gaming)
	{
		main_window->s_renderer->pending_update = true;

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
									auto id = c_main->cell(ivec2(x, j - 1));
									c_main->set_cell(uvec2(x, j), id, id == TileGrid ? cvec4(255) : mino_col_decay);
								}
							}
							else
							{
								for (auto x = 0; x < board_width; x++)
									c_main->set_cell(uvec2(x, j), TileGrid);
							}
						}
					}
				}

				clear_ticks = -1;
			}
		}
		else
		{
			if (mino_pos.y >= 0)
			{
				draw_mino(c_main, TileGrid, mino_pos, 0, mino_coords);
				if (mino_bottom_dist > 0)
					draw_mino(c_main, TileGrid, mino_pos, mino_bottom_dist, mino_coords);
			}

			if (mino_pos.y < 0)
			{
				if (mino_pos.y == -1 || mino_type == MinoTypeCount)
				{
					mino_type = mino_packs[mino_pack_idx.x][mino_pack_idx.y++];
					if (mino_pack_idx.y >= MinoTypeCount)
					{
						shuffle_pack(mino_pack_idx.x);
						mino_pack_idx = ivec2(1 - mino_pack_idx.x, 0);
					}
					for (auto i = 0; i < size(c_next); i++)
					{
						c_next[i]->clear_cells();
						auto next_idx = mino_pack_idx;
						next_idx.y += i;
						if (next_idx.y >= MinoTypeCount)
						{
							next_idx.x = 1 - next_idx.x;
							next_idx.y %= MinoTypeCount;
						}
						auto t = mino_packs[next_idx.x][next_idx.y];
						ivec2 coords[3];
						for (auto j = 0; j < 3; j++)
							coords[j] = g_mino_coords[t][j];
						draw_mino(c_next[i], TileMino1 + t, ivec2(1), 0, coords);
					}
				}
				if (mino_pos.y == -2)
				{
					c_hold->clear_cells();
					if (mino_hold != MinoTypeCount)
					{
						ivec2 coords[3];
						for (auto i = 0; i < 3; i++)
							coords[i] = g_mino_coords[mino_hold][i];
						draw_mino(c_hold, TileMino1 + mino_hold, ivec2(1), 0, coords);
					}
				}
				mino_pos = ivec2(4, 5 - (mino_type == Mino_I ? 1 : 0));
				mino_rotation = 0;
				for (auto i = 0; i < 3; i++)
					mino_coords[i] = g_mino_coords[mino_type][i];
				mino_reset_times = -1;
				mino_ticks = 0;

				dead = !check_board(c_main, ivec2(0));
				if (dead)
				{
					{
						auto pos = mino_pos;
						c_main->set_cell(uvec2(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
					}
					{
						auto pos = mino_pos + mino_coords[0];
						c_main->set_cell(uvec2(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
					}
					{
						auto pos = mino_pos + mino_coords[1];
						c_main->set_cell(uvec2(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
					}
					{
						auto pos = mino_pos + mino_coords[2];
						c_main->set_cell(uvec2(pos), c_main->cell(pos) == TileGrid ? TileMino1 + mino_type : TileGray, mino_col_decay);
					}
				}
				if (dead || (game_mode == GameSingleRTA && lines >= 40))
				{
					gaming = false;

					if (game_mode != GameVS)
					{
						auto layer = ui.e_begin_dialog()->parent;
						ui.e_text(L"Game Over");
						ui.c_aligner(AlignMiddle, 0);
						ui.e_text((L"Time: " + wfmt(L"%02d:%02d", (int)play_time / 60, ((int)play_time) % 60)).c_str());
						ui.e_text((L"Level: " + wfmt(L"%d", level)).c_str());
						ui.e_text((L"Lines: " + wfmt(L"%d", lines)).c_str());
						ui.e_text((L"Score: " + wfmt(L"%d", score)).c_str());
						ui.e_button(L"Quit", [](Capture& c) {
							remove_layer(c.thiz<Entity>());
							g_app.quit_game();
						}, Capture().set_thiz(layer));
						ui.c_aligner(AlignMiddle, 0);
						ui.e_button(L"Restart", [](Capture& c) {
							remove_layer(c.thiz<Entity>());
							g_app.start_game();
						}, Capture().set_thiz(layer));
						ui.c_aligner(AlignMiddle, 0);
						ui.e_end_dialog();
					}
				}
			}

			if (!dead)
			{
				if (key_states[key_map[KEY_HOLD]] == (KeyStateDown | KeyStateJust) && (game_mode == GameSinglePractice || mino_just_hold == false))
				{
					mino_pos.y = -2;
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
						ivec2 new_coords[3];
						ivec2 offset;
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
					if (mx != 0 && check_board(c_main, ivec2(mx, 0)))
					{
						mino_pos.x += mx;
						moved = true;

						sound_move_src->play();
					}

					if (!last_is_rotate_action)
						last_is_rotate_action = rotated && !moved;
					else
						last_is_rotate_action = !moved;

					mino_bottom_dist = 0;
					while (check_board(c_main, ivec2(0, mino_bottom_dist + 1)))
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
							mino_pos.y += mino_bottom_dist;
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
										auto pos = board_element->global_pos + vec2(board_element->padding[0], board_element->padding[1]);
										pos.y += i * cell_size.y;
										ui.parents.push(main_window->root);
										ui.next_element_pos = pos;
										ui.next_element_size = vec2(cell_size.x * board_width, cell_size.y);
										ui.e_empty();
										auto element = ui.c_element();
										element->color = cvec4(255);
										ui.parents.pop();

										struct Capturing
										{
											cElement* e;
											uint f;
										}capture;
										capture.e = element;
										capture.f = 5;
										looper().add_event([](Capture& c) {
											auto& capture = c.data<Capturing>();
											capture.f--;
											if (capture.f > 0)
											{
												capture.e->pos.x -= 10.f;
												capture.e->size.x += 20.f;
												capture.e->pos.y += 2.4f;
												capture.e->size.y -= 4.8f;
												capture.e->color.a = max(capture.e->color.a - 30, 0);
												c._current = nullptr;
											}
											else
											{
												auto e = capture.e->entity;
												e->parent->remove_child(e);
											}
										}, Capture().set_data(&capture), 0.f);
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
									ivec2 judge_points[] = {
										ivec2(-1, -1),
										ivec2(+1, -1),
										ivec2(-1, +1),
										ivec2(+1, +1),
									};
									auto count = 0;
									for (auto i = 0; i < size(judge_points); i++)
									{
										auto p = mino_pos + judge_points[i];
										if (p.x < 0 || p.x >= board_width ||
											p.y < 0 || p.y >= board_height ||
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
									for (auto it = garbages.begin(); it != garbages.end();)
									{
										if (it->lines <= cancel)
										{
											attack -= it->lines;
											cancel -= it->lines;
											it = garbages.erase(it);
										}
										else
										{
											it->lines -= cancel;
											attack -= cancel;
											cancel = 0;
											break;
										}
									}
									need_update_garbages_tip = true;
								}

								if (!special_str.empty())
								{
									c_text_special->entity->set_visible(true);
									c_text_special->set_text(special_str.c_str());
									looper().remove_events(FLAME_CHASH("special_text"));
									looper().add_event([](Capture&) {
										g_app.c_text_special->entity->set_visible(false);
									}, Capture(), 1.f, FLAME_CHASH("special_text"));
								}

								if (game_mode == GameVS && attack > 0)
								{
									nlohmann::json req;
									req["action"] = "attack";
									req["value"] = attack;
									auto n = rand() % players.size() + 1;
									auto target = -1;
									while (n > 0)
									{
										target++;
										if (target == players.size())
											target = 0;
										if (target != my_room_index)
										{
											auto& p = players[target];
											if (p.id && !p.disconnected)
												n--;
										}
									}
									if (server)
									{
										req["index"] = my_room_index;
										auto str = req.dump();
										auto& p = players[target];
										if (p.id && !p.disconnected)
											server->send(p.id, str.data(), str.size(), false);
									}
									if (client)
									{
										req["target"] = target;
										auto str = req.dump();
										client->send(str.data(), str.size());
									}
								}

								for (auto i = 0; i < l; i++)
								{
									for (auto x = 0; x < board_width; x++)
										c_main->set_cell(uvec2(x, full_lines[i]), TileGrid);
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
								for (auto it = garbages.begin(); it != garbages.end();)
								{
									if (it->time == 0)
									{
										auto n = it->lines;
										for (auto i = 0; i < board_height - n; i++)
										{
											for (auto x = 0; x < board_width; x++)
											{
												auto id = c_main->cell(ivec2(x, i + n));
												c_main->set_cell(uvec2(x, i), id, id == TileGrid ? cvec4(255) : mino_col_decay);
											}
										}
										auto hole = rand() % board_width;
										for (auto i = 0; i < n; i++)
										{
											auto y = board_height - i - 1;
											for (auto x = 0; x < board_width; x++)
												c_main->set_cell(uvec2(x, y), TileGray, mino_col_decay);
											c_main->set_cell(uvec2(hole, y), TileGrid);
										}
										it = garbages.erase(it);
										need_update_garbages_tip = true;
									}
									else
										it++;
								}

								clear_ticks = 0;
								combo = 0;

								if (hard_drop)
									sound_hard_drop_src->play();
								else
									sound_soft_drop_src->play();
							}
							mino_pos.y = -1;
							mino_just_hold = false;
						}
						else
						{
							mino_pos.y++;
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

					if (mino_pos.y != -1)
					{
						if (mino_bottom_dist)
							draw_mino(c_main, TileGhost, mino_pos, mino_bottom_dist, mino_coords, g_mino_colors[mino_type]);
						draw_mino(c_main, TileMino1 + mino_type, mino_pos, 0, mino_coords);
					}
				}
			}

			if (game_mode == GameVS)
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
						auto id = c_main->cell(ivec2(x, y));
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
		if (game_mode == GameVS)
		{
			if (need_update_garbages_tip)
			{
				e_garbage->remove_children(0, -1);
				ui.parents.push(e_garbage);
				auto idx = 0;
				for (auto i = 0; i < garbages.size(); i++)
				{
					auto& g = garbages[i];
					for (auto j = 0; j < g.lines; j++)
					{
						ui.next_element_pos = vec2(0.f, -idx * 24.f - i * 4.f);
						ui.next_element_size = vec2(24.f);
						ui.e_image((atlas->canvas_slot_ << 16) + atlas->find_tile("gray.png"));
						idx++;
					}
				}
				ui.parents.pop();
				need_update_garbages_tip = false;
			}
			auto idx = 0;
			for (auto i = 0; i < garbages.size(); i++)
			{
				auto& g = garbages[i];
				if (g.time > 0)
					g.time--;
				if (g.time == 0)
				{
					for (auto j = 0; j < g.lines; j++)
						e_garbage->children[idx + j]->get_component(cImage)->color = cvec4(255, 0, 0, 255);
				}
				idx += g.lines;
			}
		}
	}
}

int main(int argc, char **args)
{
	g_app.create();

	new MainForm();

	g_app.run();

	return 0;
}
