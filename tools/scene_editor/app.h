#pragma once

#include <flame/graphics/application.h>

#include <ImFileDialog.h>

using namespace flame;

struct Window
{
	std::string name;

	void* lis = nullptr;

	Window(std::string_view name);

	void open();
	void close();
	void draw();

	virtual void on_draw() = 0;
};

struct App : GraphicsApplication
{
	std::filesystem::path project_path;

	bool always_update = false;

	void init();

	void open_project(const std::filesystem::path& path);
};

extern App app;
