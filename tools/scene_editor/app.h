#pragma once

#include <flame/universe/application.h>

using namespace flame;

struct View
{
	std::string name;

	void* lis = nullptr;

	View(std::string_view name);

	void open();
	void close();
	void draw();

	virtual void init() {}
	virtual void on_draw() = 0;
};

struct App : UniverseApplication
{
	enum DialogType
	{
		DialogNone,
		DialogMessage,
		DialogInput
	};

	std::filesystem::path project_path;
	std::filesystem::path prefab_path;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;

	bool open_dialog = false;
	DialogType dialog_type = DialogNone;
	std::string dialog_title;
	std::string dialog_text;
	std::function<void(bool, const std::string&)> dialog_callback;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);

	bool cmd_create_entity(EntityPtr dst = nullptr/* entity or nullptr to use e_prefab */, uint type = "empty"_h);
	bool cmd_delete_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
	bool cmd_play();
	bool cmd_stop();

	void close_dialog();
	void open_message_dialog(const std::string& title, const std::string& content = "");
	void open_input_dialog(const std::string& title, const std::function<void(bool, const std::string&)>& callback);
};

extern App app;

PrefabInstance* get_prefab_instance(EntityPtr e);
