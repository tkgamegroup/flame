#pragma once

#include "app.h"

struct WindowInspector : View
{
	WindowInspector();

	void on_draw() override;
};

extern WindowInspector window_inspector;
