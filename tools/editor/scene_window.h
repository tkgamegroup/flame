#pragma once

#include "app.h"

enum Tool
{
	ToolNone,
	ToolSelect,
	ToolMove,
	ToolRotate,
	ToolScale,
	ToolTerrainUp,
	ToolTerrainDown,
	ToolTerrainSmooth,
	ToolTerrainPaint,
	ToolTileMapLevelUp,
	ToolTileMapLevelDown,
	ToolTileMapSlope,
	ToolTileMapFlat,
};

enum ToolPivot
{
	ToolIndividual,
	ToolCenter
};

enum ToolMode
{
	ToolLocal,
	ToolWorld
};

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

	Tool tool = ToolSelect;
	ToolPivot tool_pivot = ToolIndividual;
	ToolMode tool_mode = ToolLocal;
	bool move_snap = true;
	float move_snap_value = 0.5f;
	float move_snap_2d_value = 4.f;
	vec3 get_snap_pos(const vec3& _pos);
	bool rotate_snap = true;
	float rotate_snap_value = 5.f;
	bool scale_snap = true;
	float scale_snap_value = 0.1f;

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
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
	SceneView* first_view() const;
	SceneView* last_focused_view() const;
};

extern SceneWindow scene_window;
