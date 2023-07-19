#pragma once

#include "app.h"

struct SceneView : View
{
	std::unique_ptr<graphics::Image> render_tar;
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

	SceneView();
	SceneView(const std::string& name);
	cCameraPtr curr_camera();
	vec3 camera_target_pos();
	void reset_camera(uint op);
	void focus_to_selected();
	void selected_to_focus();
	void on_draw() override;
};

struct SceneWindow : Window
{
	bool fixed_render_target_size = false;
	std::string last_focused_view_name;

	SceneWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
	SceneView* first_view() const;
	SceneView* last_focused_view() const;
};

extern SceneWindow scene_window;
