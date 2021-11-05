#pragma once

#include "app.h"

struct WindowScene : Window
{
	graphics::Image* render_tar = nullptr;

	WindowScene();

	void on_draw() override;
};

extern WindowScene window_scene;
