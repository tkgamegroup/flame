#pragma once

#include "app.h"

struct WindowConsole : Window
{
	WindowConsole();

	void on_draw() override;
};

extern WindowConsole window_console;
