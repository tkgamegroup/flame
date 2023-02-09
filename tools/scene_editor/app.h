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

struct History
{
	virtual ~History() {}

	virtual void undo() = 0;
	virtual void redo() = 0;
};

struct AssetModifyHistory : History
{
	std::filesystem::path path;
	uint asset_hash;
	uint attr_hash;
	std::string old_value;
	std::string new_value;

	AssetModifyHistory(const std::filesystem::path& path, uint asset_hash, uint attr_hash,
		const std::string& old_value, const std::string& new_value);
	void set_value(const std::string& value);
	void undo() override;
	void redo() override;
};

struct EntityModifyHistory : History
{
	std::string guid;
	uint comp_hash;
	uint attr_hash;
	std::string old_value;
	std::string new_value;

	EntityModifyHistory(const std::string& guid, uint comp_hash, uint attr_hash, 
		const std::string& old_value, const std::string& new_value);
	void set_value(const std::string& value);
	void undo() override;
	void redo() override;
};

extern int history_idx;
extern std::vector<std::unique_ptr<History>> histories;
inline void add_history(History* h)
{
	if (history_idx + 1 < histories.size())
		histories.erase(histories.begin() + history_idx + 1, histories.end());
	histories.emplace_back(h);
	history_idx++;
}

struct EditingObject
{
	virtual ~EditingObject() {}

	virtual int type() = 0;
};

struct EditingAsset : EditingObject
{
	std::filesystem::path path; 
	uint asset_hash;

	EditingAsset(const std::filesystem::path& path, uint asset_hash);
	
	int type() override { return 0; }
};

struct EditingEntity : EditingObject
{
	EditingEntity(const std::string& guid);

	std::string guid;

	int type() override { return 1; }
};

struct EditingComponent : EditingObject
{
	EditingComponent(const std::string& guid, uint comp_hash);

	std::string guid;
	uint comp_hash;

	int type() override { return 2; }
};

extern std::stack<std::unique_ptr<EditingObject>> editing_objects;

struct App : UniverseApplication
{
	bool graphics_debug = true;
	std::vector<std::pair<uint, uint>> graphics_configs;

	std::filesystem::path project_path;
	std::filesystem::path prefab_path;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;
	EntityPtr e_preview = nullptr;
	bool paused = false;
	bool control = true;

	Tool tool = ToolSelect;
	ToolMode tool_mode = ToolLocal;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);
	void new_prefab(const std::filesystem::path& path);

	bool cmd_undo();
	bool cmd_redo();
	bool cmd_create_entity(EntityPtr dst = nullptr/* entity or nullptr to use e_prefab */, uint type = "empty"_h);
	bool cmd_delete_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
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
