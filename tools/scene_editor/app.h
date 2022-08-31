#pragma once

#include <flame/universe/application.h>

using namespace flame;

enum Tool
{
	ToolNone,
	ToolSelect,
	ToolMove,
	ToolRotate,
	ToolScale,
	ToolTerrainUp,
	ToolTerrainDown,
	ToolTerrainPaint
};

enum ToolMode
{
	ToolLocal,
	ToolWorld
};

struct App : UniverseApplication
{
	std::filesystem::path project_path;
	std::filesystem::path prefab_path;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;
	bool paused = false;

	Tool tool = ToolSelect;
	ToolMode tool_mode = ToolLocal;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);
	void new_prefab(const std::filesystem::path& path);

	bool cmd_create_entity(EntityPtr dst = nullptr/* entity or nullptr to use e_prefab */, uint type = "empty"_h);
	bool cmd_delete_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
	bool cmd_play();
	bool cmd_pause();
	bool cmd_stop();

	void open_message_dialog(const std::string& title, const std::string& message);
};

extern App app;

PrefabInstance* get_prefab_instance(EntityPtr e);
