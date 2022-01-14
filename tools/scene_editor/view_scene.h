#pragma once

#include "app.h"

struct View_Scene : View
{
	graphics::Image* render_tar = nullptr;

	cNodePtr camera_node = nullptr;
	float camera_zoom = 2.f;

	View_Scene();

	void on_draw() override;
};

extern View_Scene view_scene;
