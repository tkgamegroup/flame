#pragma once

#include "app.h"

struct WindowProject : Window
{
	WindowProject();

	void on_draw() override;
};

extern WindowProject window_project;
