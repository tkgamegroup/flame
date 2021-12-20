#pragma once

#include "app.h"

struct View_Console : View
{
	View_Console();

	void on_draw() override;
};

extern View_Console view_console;
