#pragma once

#include "app.h"

struct ConsoleView : View
{
	ConsoleView();
	ConsoleView(const std::string& name);
	~ConsoleView();
	void on_draw() override;
};

struct ConsoleWindow : Window
{
	ConsoleWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern ConsoleWindow console_window;
