#pragma once

#include "app.h"

struct WindowHierarchy : Window
{
	WindowHierarchy();

	bool _just_selected;

	void on_draw() override;
};

extern WindowHierarchy window_hierarchy;
