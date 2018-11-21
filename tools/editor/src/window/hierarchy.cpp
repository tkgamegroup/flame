#include "../select.h"
#include "scene_editor.h"
#include "hierarchy.h"
#include "resource_explorer.h"

HierarchyWindow *hierarchy_window = nullptr;

HierarchyWindow::HierarchyWindow() :
	Window("Hierarchy")
{
}

HierarchyWindow::~HierarchyWindow()
{
	hierarchy_window = nullptr;
}

static void show_nodes(flame::Node *n)
{
	auto node_style = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (n == selected.get_node())
		node_style |= ImGuiTreeNodeFlags_Selected;
	if (n->get_children().size() == 0)
		node_style |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	auto node_opened = ImGui::TreeNodeEx(n->name.c_str(), node_style);
	if (ImGui::IsItemClicked())
		selected = n;
	if (n->get_children().size() > 0 && node_opened)
	{
		for (auto &c : n->get_children())
			show_nodes(c.get());
		ImGui::TreePop();
	}
}

void HierarchyWindow::on_show()
{
	if (scene_editor)
	{
		auto scene = scene_editor->scene;
		show_nodes(scene);
	}
}
