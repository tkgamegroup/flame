#pragma once

#include "app.h"

#include <flame/foundation/blueprint.h>
#if USE_IMGUI_NODE_EDITOR
#include <flame/graphics/gui.h>
#endif

struct BlueprintView : View
{
#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::Detail::EditorContext* ax_node_editor = nullptr;
#endif
	std::filesystem::path blueprint_path;
	BlueprintPtr blueprint = nullptr;
	BlueprintInstancePtr blueprint_instance = nullptr;
	std::string group_name = "main";
	uint group_name_hash = "main"_h;
	uint load_frame = 0;
	BlueprintGroupPtr last_group = nullptr;
	bool unsaved = false;

	BlueprintView();
	BlueprintView(const std::string& name);
	~BlueprintView();

	void process_relationships(BlueprintNodePtr n);
	void expand_block_sizes();
	void copy_nodes(BlueprintGroupPtr g);
	void paste_nodes(BlueprintGroupPtr g, const vec2& pos);
	void set_parent_to_last_node();
	void run_blueprint(BlueprintInstance::Group* debugging_group);
	void step_blueprint(BlueprintInstance::Group* debugging_group);
	void stop_blueprint(BlueprintInstance::Group* debugging_group);
	void save_blueprint();
	void on_draw() override;
	std::string get_save_name() override;
};

struct BlueprintWindow : Window
{
	BlueprintDebuggerPtr debugger = nullptr;

	BlueprintWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern BlueprintWindow blueprint_window;
