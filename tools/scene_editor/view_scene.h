#pragma once

#include "app.h"

struct View_Scene : View
{
	std::unique_ptr<graphics::Image> render_tar;

	bool show_AABB = false;
	bool show_axis = true;
	bool show_bones = false;

	uint camera_idx = 0;
	float camera_zoom = 5.f;
	cNodePtr hovering_node = nullptr;
	vec3 hovering_pos;

	View_Scene();

	cCameraPtr curr_camera();
	void focus_to_selected();
	void selected_to_focus();
	void on_draw() override;
};

extern View_Scene view_scene;
