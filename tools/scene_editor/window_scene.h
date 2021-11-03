#pragma once

#include "app.h"

struct WindowScene : Window
{
	WindowScene();

	void on_draw() override;
};

extern WindowScene window_scene;
