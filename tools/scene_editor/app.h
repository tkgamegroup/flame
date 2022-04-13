#pragma once

#include <flame/universe/application.h>

using namespace flame;

enum Icon
{
	Icon_Model,
	Icon_Armature,
	Icon_Mesh,

	IconCount
};

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
	std::filesystem::path project_path;
	std::filesystem::path prefab_path;
	EntityPtr e_editor = nullptr;
	EntityPtr e_prefab = nullptr;
	EntityPtr e_playing = nullptr;

	graphics::ImagePtr icons[IconCount];
	std::map<int, std::unique_ptr<graphics::Image>> sys_icons;

	void init();

	void new_project(const std::filesystem::path& path);
	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);

	bool cmd_create_entity(EntityPtr dst = nullptr/* entity or nullptr to use e_prefab */, uint type = "empty"_h);
	bool cmd_delete_entity(EntityPtr e = nullptr/* entity or nullptr to use selected entity */);
	bool cmd_play();
	bool cmd_stop();
};

extern App app;

PrefabInstance* get_prefab_instance(EntityPtr e);
