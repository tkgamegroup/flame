#include "selection.h"
#include "view_hierarchy.h"
#include "view_scene.h"

View_Hierarchy view_hierarchy;

View_Hierarchy::View_Hierarchy() :
	View("Hierarchy")
{
}

uint hierarchy_select_frame = 0;

void View_Hierarchy::on_draw()
{
	auto just_select = selection.frame == (int)frames - 1 && selection.type == Selection::tEntity;
	if (hierarchy_select_frame == selection.frame)
		just_select = false;

	std::vector<EntityPtr> open_nodes;
	if (just_select)
	{
		auto e = selection.entity()->parent;
		while (e)
		{
			open_nodes.push_back(e);
			e = e->parent;
		}
	}

	std::function<void(EntityPtr, bool)> draw_entity;
	draw_entity = [&](EntityPtr e, bool in_prefab) {
		std::function<bool(EntityPtr, EntityPtr)> is_ancestor;
		is_ancestor = [&](EntityPtr t, EntityPtr e) {
			if (!e->parent)
				return false;
			if (e->parent == t)
				return true;
			return false;
		};

		auto flags = selection.selecting(e) ? ImGuiTreeNodeFlags_Selected : 0;
		if (e->children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		else
			flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		ImGui::PushID(e);
		if (std::find(open_nodes.begin(), open_nodes.end(), e) != open_nodes.end())
			ImGui::SetNextItemOpen(true);

		if (e->prefab)
			in_prefab = true;
		auto name = e->name;
		if (!in_prefab)
			name = "[] " + name;
		else
			name = "[-] " + name;
		if (in_prefab)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.8f, 1.f, 1.f));
		auto opened = ImGui::TreeNodeEx(name.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
		if (just_select && selection.selecting(e))
			ImGui::SetScrollHereY();
		if (in_prefab)
			ImGui::PopStyleColor();
		ImGui::PopID();

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("Entity", &e, sizeof(void*));
			ImGui::TextUnformatted("Entity");
			ImGui::EndDragDropSource();
		}

		auto read_drop_entity = [&]()->EntityPtr {
			if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
			{
				if (get_prefab_instance(e))
				{
					ImGui::OpenMessageDialog("[RestructurePrefabInstanceWarnning]", "");
					return nullptr;
				}
				else
				{
					auto e_src = *(EntityPtr*)payload->Data;
					if (!e_src->prefab && get_prefab_instance(e_src))
					{
						ImGui::OpenMessageDialog("[RestructurePrefabInstanceWarnning]", "");
						return nullptr;
					}
					if (!is_ancestor(e_src, e))
						return e_src;
				}
			}
			return nullptr;
		};
		auto read_drop_file = [&]()->EntityPtr {
			if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
			{
				if (get_prefab_instance(e))
				{
					ImGui::OpenMessageDialog("[RestructurePrefabInstanceWarnning]", "");
					return nullptr;
				}
				else
				{
					auto str = std::wstring((wchar_t*)payload->Data);
					auto path = Path::reverse(str);
					if (path.extension() == L".prefab")
					{
						auto e_src = Entity::create();
						e_src->load(path);
						new PrefabInstance(e_src, path);
						return e_src;
					}
				}
			}
			return nullptr;
		};

		if (ImGui::BeginDragDropTarget())
		{
			if (auto e_src = read_drop_entity(); e_src)
			{
				e_src->parent->remove_child(e_src, false);
				e->add_child(e_src);
			}
			if (auto e_src = read_drop_file(); e_src)
				e->add_child(e_src);
			ImGui::EndDragDropTarget();
		}
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::IsItemHovered())
		{
			selection.select(e);
			hierarchy_select_frame = frames;
		}
		if (opened)
		{
			auto gap_item = [&](int i) {
				ImGui::PushID(e); ImGui::PushID(i);
				ImGui::InvisibleButton("gap", ImVec2(-1, 4));
				ImGui::PopID(); ImGui::PopID();
				if (ImGui::BeginDragDropTarget())
				{
					if (auto e_src = read_drop_entity(); e_src)
					{
						auto idx = i;
						if (e_src->parent == e && e_src->index < i) idx--;
						e_src->parent->remove_child(e_src, false);
						e->add_child(e_src, idx);
					}
					if (auto e_src = read_drop_file(); e_src)
						e->add_child(e_src, i);
					ImGui::EndDragDropTarget();
				}
			};
			auto i = 0;
			for (; i < e->children.size(); i++)
			{
				gap_item(i);
				draw_entity(e->children[i].get(), in_prefab);
			}
			gap_item(i);
			ImGui::TreePop();
		}
	};

	auto e = app.e_playing;
	if (!e)
		e = app.e_prefab;
	if (e)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.f));
		draw_entity(e, 0);
		ImGui::PopStyleVar(1);
	}

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		if (ImGui::IsKeyPressed(Keyboard_Del))
			app.cmd_delete_entity();
		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && selection.frame != frames)
			selection.clear();
	}
}
