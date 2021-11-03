#pragma once

#include "app.h"

struct WindowInspector : Window
{
	WindowInspector();

	void on_draw() override;
};

extern WindowInspector window_inspector;
