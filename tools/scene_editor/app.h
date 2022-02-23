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

	virtual void on_draw() = 0;
};

struct App : UniverseApplication
{
	std::filesystem::path project_path;

	std::filesystem::path prefab_path;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_editor = nullptr;

	bool always_update = false;

	bool open_message_dialog = false;
	std::string message_dialog_title;
	std::string message_dialog_text;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);

	bool cmd_create_entity();
	bool cmd_delete_selected_entity();

	void show_message_dialog(const std::string& title, const std::string& content = "");
};

extern App app;

PrefabInstance* get_prefab_instance(EntityPtr e);
