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
	bool unsaved = false;

	BlueprintView();
	BlueprintView(const std::string& name);
	~BlueprintView();

	void load_blueprint(const std::filesystem::path& path);
	void process_object_moved(BlueprintObject obj);
	void expand_block_sizes();
	void on_draw() override;
};

struct BlueprintWindow : Window
{
	BlueprintDebuggerPtr debugger = nullptr;

	BlueprintWindow();
	View* open_view(bool new_instance) override;
	View* open_view(const std::string& name) override;
};

extern BlueprintWindow blueprint_window;
