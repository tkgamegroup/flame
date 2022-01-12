#pragma once

#include "app.h"

struct View_Scene : View
{
	graphics::Image* render_tar = nullptr;

	View_Scene();

	void on_draw() override;
};

extern View_Scene view_scene;
