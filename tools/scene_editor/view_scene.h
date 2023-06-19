#pragma once

#include "app.h"

struct View_Scene : graphics::GuiView
{
	std::unique_ptr<graphics::Image> render_tar;
	bool fixed_render_target_size = false;

	bool show_outline = true;
	bool show_AABB = false;
	bool show_AABB_only_selected = false;
	bool show_axis = true;
	bool show_bones = false;
	bool show_navigation = false;
	uint show_navigation_frames = 0;

	uint camera_idx = 0;
	float camera_zoom = 5.f;
	EntityPtr hovering_entity = nullptr;
	vec3 hovering_pos;
	std::vector<cNodePtr> node_targets;
	std::vector<cElementPtr> element_targets;

	View_Scene();

	cCameraPtr curr_camera();
	vec3 camera_target_pos();
	void reset_camera(uint op);
	void focus_to_selected();
	void selected_to_focus();
	void on_draw() override;
	bool on_begin() override;
};

extern View_Scene view_scene;
