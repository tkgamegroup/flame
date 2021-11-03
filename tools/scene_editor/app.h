#pragma once

#include <flame/universe/app.h>

using namespace flame;

struct Window
{
	std::string name;

	Entity* e = nullptr;

	Window(const std::string& name);

	void open();
	void close();
	void draw();

	virtual void on_draw() = 0;
};

struct MyApp : App
{
	Entity* imgui_root = nullptr;

	void init();
};

extern MyApp app;
