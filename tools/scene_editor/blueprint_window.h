#pragma once

#include "app.h"

#if USE_IMGUI_NODE_EDITOR
#include <flame/graphics/gui.h>
#endif

struct BlueprintView : View
{
#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::EditorContext* im_editor = nullptr;
#endif
	BlueprintPtr blueprint = nullptr;
	BlueprintInstancePtr blueprint_instance = nullptr;

	BlueprintView();
	BlueprintView(const std::string& name);
	void on_draw() override;
};

struct BlueprintWindow : Window
{
	BlueprintDebuggerPtr debugger = nullptr;

	BlueprintWindow();
	void open_view(bool new_instance) override;
	void open_view(const std::string& name) override;
};

extern BlueprintWindow blueprint_window;
