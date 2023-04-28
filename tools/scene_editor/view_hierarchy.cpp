#include "selection.h"
#include "view_hierarchy.h"
#include "view_scene.h"

View_Hierarchy view_hierarchy;
static auto selection_changed = false;

View_Hierarchy::View_Hierarchy() :
	GuiView("Hierarchy")
{
	selection.callbacks.add([](uint caller) {
		if (caller != "hierarchy"_h)
			selection_changed = true;
	}, "hierarchy"_h);
}

struct Entities
{
	EntityPtr* p;
	uint n;
};

void View_Hierarchy::on_draw()
{
	EntityPtr select_entity = nullptr;
	EntityPtr focus_entity = selection_changed ? (selection.type == Selection::tEntity ? selection.as_entity(-1) : nullptr) : nullptr;

	std::vector<EntityPtr> open_nodes;
	if (focus_entity)
	{
		auto e = focus_entity;
		while (e)
		{
			open_nodes.push_back(e);
			e = e->parent;
		}
	}

	std::function<void(EntityPtr, bool)> show_entity;
	show_entity = [&](EntityPtr e, bool in_prefab) {
		auto flags = selection.selecting(e) ? ImGuiTreeNodeFlags_Selected : 0;
		if (e->children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		else
			flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		if (!open_nodes.empty())
		{
			if (std::find(open_nodes.begin(), open_nodes.end(), e) != open_nodes.end())
				ImGui::SetNextItemOpen(true);
		}

		if (e->prefab_instance)
			in_prefab = true;
		auto display_name = e->name;
		if (!in_prefab)
			display_name = "[] " + display_name;
		else
			display_name = "[-] " + display_name;
		display_name += "###";
		display_name += str((uint64)e);
		if (in_prefab)
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.8f, 1.f, 1.f));
		auto opened = ImGui::TreeNodeEx(display_name.c_str(), flags) && !(flags & ImGuiTreeNodeFlags_Leaf);
		if (e == focus_entity)
			ImGui::SetScrollHereY();
		if (in_prefab)
			ImGui::PopStyleColor();

		if (ImGui::BeginDragDropSource())
		{
			auto found = false;
			for (auto _e : selection.get_entities())
			{
				if (e == _e)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				ImGui::SetDragDropPayload("Entity", &e, sizeof(void*));
				ImGui::TextUnformatted(e->name.c_str());
				ImGui::EndDragDropSource();
			}
			else
			{
				Entities es;
				es.p = (EntityPtr*)selection.objects.data();
				es.n = selection.objects.size();
				ImGui::SetDragDropPayload("Entities", &es, sizeof(Entities));
				ImGui::Text("%d entities", es.n);
				ImGui::EndDragDropSource();
			}
		}

		auto read_drop_entities = [e]()->std::vector<EntityPtr> {
			std::vector<EntityPtr> ret;
			if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
			{
				if (get_root_prefab_instance(e))
				{
					app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
					return ret;
				}
				auto e = *(EntityPtr*)payload->Data;
				if (!e->prefab_instance && get_root_prefab_instance(e))
				{
					app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
					return ret;
				}
				if (is_ancestor(e, e))
					return ret;
				ret.push_back(e);
			}
			if (auto payload = ImGui::AcceptDragDropPayload("Entities"); payload)
			{
				if (get_root_prefab_instance(e))
				{
					app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
					return ret;
				}
				auto& es = *(Entities*)payload->Data;
				for (auto i = 0; i < es.n; i++)
				{
					auto e = es.p[i];
					if (!e->prefab_instance && get_root_prefab_instance(e))
					{
						app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
						return ret;
					}
				}
				for (auto i = 0; i < es.n; i++)
				{
					for (auto j = 0; j < es.n; j++)
					{
						if (is_ancestor(es.p[i], es.p[j]))
						{
							app.open_message_dialog("Cannot reparent", "The entities you select must not have parentships");
							return ret;
						}
					}
				}
				for (auto i = 0; i < es.n; i++)
				{
					if (is_ancestor(es.p[i], e))
						return ret;
				}
				for (auto i = 0; i < es.n; i++)
					ret.push_back(es.p[i]);
			}
			if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
			{
				if (get_root_prefab_instance(e))
				{
					app.open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
					return ret;
				}
				auto str = std::wstring((wchar_t*)payload->Data);
				auto path = Path::reverse(str);
				if (path.extension() == L".prefab")
				{
					auto e = Entity::create(path);
					new PrefabInstance(e, path);
					ret.push_back(e);
				}
			}
			return ret;
		};

		if (ImGui::BeginDragDropTarget())
		{
			auto es = read_drop_entities();
			if (!es.empty())
			{
				for (auto _e : es)
				{
					if (_e->parent)
						_e->remove_from_parent(false);
				}
				for (auto _e : es)
					e->add_child(_e);
				app.prefab_unsaved = true;
			}
			ImGui::EndDragDropTarget();
		}

		if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && ImGui::IsItemHovered())
			select_entity = e;

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Duplicate"))
				app.cmd_duplicate_entities(selection.get_entities());
			if (ImGui::MenuItem("Delete"))
				app.cmd_delete_entities(selection.get_entities());
			ImGui::EndPopup();
		}

		if (opened)
		{
			auto gap_item = [&](int i) {
				ImGui::PushID(i);
				ImGui::InvisibleButton("gap", ImVec2(-1, 2));
				ImGui::PopID();
				if (ImGui::BeginDragDropTarget())
				{
					auto es = read_drop_entities();
					if (!es.empty())
					{
						auto idx = i;
						for (auto _e : es)
						{
							if (_e->parent == e && _e->index < i)
								idx--;
							if (_e->parent)
								_e->remove_from_parent(false);
						}
						for (auto _e : es)
						{
							e->add_child(_e, idx);
							idx++;
						}
						app.prefab_unsaved = true;
					}
					ImGui::EndDragDropTarget();
				}
			};
			auto i = 0;
			for (; i < e->children.size(); i++)
			{
				gap_item(i);
				show_entity(e->children[i].get(), in_prefab);
			}
			gap_item(i);
			ImGui::TreePop();
		}
	};

	if (auto root = app.e_playing ? app.e_playing : app.e_prefab; root)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 0.f));
		show_entity(root, false);
		ImGui::PopStyleVar(1);
	}

	if (select_entity)
	{
		if (ImGui::IsKeyDown(Keyboard_Ctrl))
		{
			auto entities = selection.get_entities();
			auto found = false;
			for (auto it = entities.begin(); it != entities.end();)
			{
				if (*it == select_entity)
				{
					found = true;
					it = entities.erase(it);
					break;
				}
				else
					it++;
			}
			if (!found)
				entities.push_back(select_entity);
			selection.select(entities, "hierarchy"_h);
		}
		else if (ImGui::IsKeyDown(Keyboard_Shift))
		{
			auto entities = selection.get_entities();
			if (entities.empty())
				selection.select(select_entity, "hierarchy"_h);
			else
			{
				auto begin = entities.front();
				auto end = select_entity;
				if (end->compare_depth(begin))
					std::swap(begin, end);
				entities.clear();

				auto root = app.e_playing ? app.e_playing : app.e_prefab;
				bool select = false;
				std::function<void(EntityPtr)> process_entity;
				process_entity = [&](EntityPtr e) {
					if (e == begin)
						select = true;
					if (select)
						entities.push_back(e);
					if (e == end)
						select = false;
					auto window = ImGui::GetCurrentWindow();
					if (!e->children.empty())
					{
						auto id = window->GetIDNoKeepAlive(("###" + str((uint64)e)).c_str());
						if (ImGui::TreeNodeBehaviorIsOpen(id))
						{
							ImGui::PushOverrideID(id);
							for (auto& c : e->children)
								process_entity(c.get());
							ImGui::PopID();
						}
					}
				};
				process_entity(root);
				selection.select(entities, "hierarchy"_h);
			}
		}
		else
			selection.select(select_entity, "hierarchy"_h);
	}

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		if (ImGui::IsKeyPressed(Keyboard_Del))
			app.cmd_delete_entities(selection.get_entities());
	}

	selection_changed = false;
}
