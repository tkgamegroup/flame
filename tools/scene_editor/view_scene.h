#pragma once

#include "app.h"

struct View_Scene : View
{
	std::unique_ptr<graphics::Image> render_tar;

	cNodePtr editor_node = nullptr;
	void* editor_drawer = nullptr;
	
	float camera_zoom = 5.f;
	cNodePtr hovering_node = nullptr;

	View_Scene();

	void on_draw() override;
};

extern View_Scene view_scene;
