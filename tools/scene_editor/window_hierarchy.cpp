#include "selection.h"
#include "window_hierarchy.h"
#include "window_scene.h"

WindowHierarchy window_hierarchy;

WindowHierarchy::WindowHierarchy() :
	Window("Hierarchy")
{
}

void WindowHierarchy::on_draw()
{
	//std::function<void(Entity*)> draw_entity;
	//draw_entity = [&](Entity* e) {
	//	auto flags = selection.selecting(e) ? ImGuiTreeNodeFlags_Selected : 0;

	//	auto n = e->get_children_count();
	//	if (n == 0)
	//		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	//	else
	//		flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	//	ImGui::PushID(e);
	//	auto opened = ImGui::TreeNodeEx(("[] " + std::string(e->get_name())).c_str(), flags);
	//	ImGui::PopID();
	//	if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())
	//	{
	//		selection.select(e);
	//		_just_selected = true;
	//	}
	//	if (opened)
	//	{
	//		for (auto i = 0; i < n; i++)
	//		{
	//			auto c = e->get_child(i);
	//			draw_entity(c);
	//			ImGui::TreePop();
	//		}
	//	}
	//};

	_just_selected = false;

	//if (window_scene.e_prefab)
	//	draw_entity(window_scene.e_prefab);

	if (ImGui::IsMouseReleased(0) && ImGui::IsWindowFocused() && !_just_selected)
		selection.clear();
}
