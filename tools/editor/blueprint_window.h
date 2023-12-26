#pragma once

#include "app.h"

#include <flame/foundation/blueprint.h>
#if USE_IMGUI_NODE_EDITOR
#include <flame/graphics/gui.h>
#endif

struct BlueprintView : View
{
#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::Detail::EditorContext* ax_editor = nullptr;
#endif
	std::filesystem::path blueprint_path;
	BlueprintPtr blueprint = nullptr;
	BlueprintInstancePtr blueprint_instance = nullptr;
	std::string group_name = "main";
	uint group_name_hash = "main"_h;
	uint load_frame = 0;
	BlueprintGroupPtr last_group = nullptr;
	uint last_block = 0;
	bool show_misc = false;
	bool hide_var_links = true;
	bool unsaved = false;

	BlueprintView();
	BlueprintView(const std::string& name);
	~BlueprintView();

	void copy_nodes(BlueprintGroupPtr g);
	void paste_nodes(BlueprintGroupPtr g, const vec2& pos);
	void set_parent_to_hovered_node();
	void navigate_to_node(BlueprintNodePtr n);
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
