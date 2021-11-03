#pragma once

#include "app.h"

struct WindowHierarchy : Window
{
	WindowHierarchy();

	void on_draw() override;
};

extern WindowHierarchy window_hierarchy;
