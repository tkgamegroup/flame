#pragma once

#include "app.h"

#include <flame/foundation/blueprint.h>
#include <flame/graphics/gui.h>

struct BlueprintView : View
{
	ax::NodeEditor::Detail::EditorContext* ax_editor = nullptr;
	std::filesystem::path	blueprint_path;
	BlueprintPtr			blueprint = nullptr;
	BlueprintInstancePtr	blueprint_instance = nullptr;
	std::string				group_name = "main";
	uint					group_name_hash = "main"_h;
	uint					load_frame = 0;
	BlueprintGroupPtr		last_group = nullptr;
	bool					show_misc = false;
	bool					hide_var_links = true;
	bool					expand_space = false;
	bool					remove_space = false;
	bool					space_clicked = false;
	Rect 					space_rect;
	bool					unsaved = false;

	std::unordered_map<uint, std::vector<vec2>> block_verts;

	BlueprintView();
	BlueprintView(const std::string& name);
	~BlueprintView();

	void copy_nodes(BlueprintGroupPtr g, bool include_children = true);
	void paste_nodes(BlueprintGroupPtr g, const vec2& pos);
	void navigate_to_node(BlueprintNodePtr n);
	void build_node_block_verts(BlueprintNodePtr n);
	void build_all_block_verts(BlueprintGroupPtr g);
	void draw_block_verts(ImDrawList* dl, BlueprintNodePtr n);
	BlueprintNodePtr find_hovered_block(BlueprintGroupPtr g, const vec2& pos);
	void run_blueprint(BlueprintInstanceGroup* debugging_group);
	void step_blueprint(BlueprintInstanceGroup* debugging_group);
	void stop_blueprint(BlueprintInstanceGroup* debugging_group);
	void save_blueprint();
	void on_draw() override;
	void on_global_shortcuts() override;
	std::string get_save_name() override;
};

struct BlueprintWindow : Window
{
	BlueprintDebuggerPtr debugger = nullptr;
	std::vector<BlueprintNodeLibraryPtr> node_libraries;

	BlueprintWindow();
	void init();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern BlueprintWindow blueprint_window;
