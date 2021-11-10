#pragma once

#include <flame/universe/app.h>

using namespace flame;

struct Window
{
	std::string name;

	Entity* e = nullptr;

	Window(std::string_view name);

	void open();
	void close();
	void draw();

	virtual void on_draw() = 0;
};

struct MyApp : App
{
	std::filesystem::path project_path;

	bool always_update = false;

	void init();

	void open_project(const std::filesystem::path& path);
};

extern MyApp app;
