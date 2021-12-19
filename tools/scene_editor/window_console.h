#pragma once

#include "app.h"

struct WindowConsole : View
{
	WindowConsole();

	void on_draw() override;
};

extern WindowConsole window_console;
