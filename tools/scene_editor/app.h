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

	void init();

	void open_project(const std::filesystem::path& path);
	void open_prefab(const std::filesystem::path& path);
};

extern App app;
