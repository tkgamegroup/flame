#pragma once

#include "app.h"

struct ConsoleView : View
{
	ConsoleView();
	ConsoleView(const std::string& name);
	void on_draw() override;
};

struct ConsoleWindow : Window
{
	ConsoleWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern ConsoleWindow console_window;
