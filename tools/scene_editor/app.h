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

enum ToolPivot
{
	ToolIndividual,
	ToolCenter
};

enum ToolMode
{
	ToolLocal,
	ToolWorld
};

struct History
{
	virtual ~History() {}

	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct AssetModifyHistory : History
{
	std::vector<std::filesystem::path> paths;
	uint asset_type;
	uint attr_hash;
	std::vector<std::string> old_values;
	std::string new_value;

	AssetModifyHistory(const std::vector<std::filesystem::path>& paths, uint asset_type, uint attr_hash,
		const std::vector<std::string>& old_values, const std::string& new_value) :
		paths(paths),
		asset_type(asset_type),
		attr_hash(attr_hash),
		old_values(old_values),
		new_value(new_value)
	{
	}

	void set_value(const std::vector<std::string>& values);
	void undo() override;
	void redo() override;
};

struct EntityModifyHistory : History
{
	std::vector<std::string> ids;
	uint comp_type;
	uint attr_hash;
	std::vector<std::string> old_values;
	std::string new_value;

	EntityModifyHistory(const std::vector<std::string>& ids, uint comp_type, uint attr_hash,
		const std::vector<std::string>& old_values, const std::string& new_value) :
		ids(ids),
		comp_type(comp_type),
		attr_hash(attr_hash),
		old_values(old_values),
		new_value(new_value)
	{
	}

	void set_value(const std::vector<std::string>& values);
	void undo() override;
	void redo() override;
};

extern int history_idx;
extern std::vector<std::unique_ptr<History>> histories;
inline void add_history(History* h)
{
	if (history_idx + 1 < histories.size())
		histories.erase(histories.begin() + (history_idx + 1), histories.end());
	histories.emplace_back(h);
	history_idx++;
}

struct App : UniverseApplication
{
	bool graphics_debug = true;
	std::vector<std::pair<uint, uint>> graphics_configs;

	std::filesystem::path project_path;
	std::filesystem::path prefab_path;
	void* project_cpp_library = nullptr;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;
	EntityPtr e_preview = nullptr;
	bool paused = false;
	bool control = true;

	Tool tool = ToolSelect;
	ToolPivot tool_pivot = ToolIndividual;
	ToolMode tool_mode = ToolLocal;
	bool move_snap = false;
	float move_snap_value = 0.5f;
	vec3 get_snap_pos(const vec3& _pos);
	bool rotate_snap = false;
	float rotate_snap_value = 5.f;
	bool scale_snap = false;
	float scale_snap_value = 0.1f;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void cmake_project();
	void build_project();
	void close_project();
	void new_prefab(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);
	bool save_prefab();
	void close_prefab();
	void load_project_cpp();
	void unload_project_cpp();
	void open_file_in_vs(const std::filesystem::path& path);
	void vs_automate(const std::vector<std::wstring>& cl);

	bool cmd_undo();
	bool cmd_redo();
	bool cmd_create_entity(EntityPtr dst = nullptr/* entity or nullptr to use e_prefab */, uint type = "empty"_h);
	bool cmd_delete_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
	bool cmd_duplicate_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
	bool cmd_play();
	bool cmd_pause();
	bool cmd_stop();
	bool cmd_start_preview();
	bool cmd_stop_preview();
	bool cmd_restart_preview();

	void open_message_dialog(const std::string& title, const std::string& message);
};

extern App app;

PrefabInstance* get_prefab_instance(EntityPtr e);
