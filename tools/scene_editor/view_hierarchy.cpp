#include "selection.h"
#include "view_hierarchy.h"
#include "view_scene.h"

View_Hierarchy view_hierarchy;

View_Hierarchy::View_Hierarchy() :
	View("Hierarchy")
{
}

void View_Hierarchy::on_draw()
{
	std::function<void(Entity*)> draw_entity;
	draw_entity = [&](Entity* e) {
		auto flags = selection.selecting(e) ? ImGuiTreeNodeFlags_Selected : 0;

		if (e->children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		else
			flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		ImGui::PushID(e);
		auto opened = ImGui::TreeNodeEx(("[] " + e->name).c_str(), flags);
		ImGui::PopID();
		if (ImGui::IsMouseReleased(0) && ImGui::IsItemHovered())
		{
			selection.select(e);
			_just_selected = true;
		}
		if (opened)
		{
			for (auto& c : e->children)
			{
				draw_entity(c.get());
				ImGui::TreePop();
			}
		}
	};

	_just_selected = false;

	if (view_scene.e_prefab)
		draw_entity(view_scene.e_prefab);

	if (ImGui::IsMouseReleased(0) && ImGui::IsWindowFocused() && !_just_selected)
		selection.clear();
}
