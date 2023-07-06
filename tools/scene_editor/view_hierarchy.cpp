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

std::vector<EntityPtr> read_drops(EntityPtr e_dst) 
{
	std::vector<EntityPtr> ret;
	if (auto payload = ImGui::AcceptDragDropPayload("Entity"); payload)
	{
		auto e = *(EntityPtr*)payload->Data;
		if (auto ins = get_root_prefab_instance(e); ins && ins != e->prefab_instance.get() && 
			ins->find_modification(e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child") == -1)
		{
			open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
			return ret;
		}
		if (is_ancestor(e, e_dst))
		{
			open_message_dialog("Cannot reparent", "The entity you select is the ancestor of the destination");
			return ret;
		}
		ret.push_back(e);
	}
	else if (auto payload = ImGui::AcceptDragDropPayload("Entities"); payload)
	{
		auto& es = *(Entities*)payload->Data;
		for (auto i = 0; i < es.n; i++)
		{
			auto e = es.p[i];
			if (auto ins = get_root_prefab_instance(e); ins && ins != e->prefab_instance.get() &&
				ins->find_modification(e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child") == -1)
			{
				open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
				return ret;
			}
		}
		for (auto i = 0; i < es.n; i++)
		{
			for (auto j = 0; j < es.n; j++)
			{
				if (is_ancestor(es.p[i], es.p[j]))
				{
					open_message_dialog("Cannot reparent", "The entities you select must not have parentships");
					return ret;
				}
			}
		}
		for (auto i = 0; i < es.n; i++)
		{
			if (is_ancestor(es.p[i], e_dst))
			{
				open_message_dialog("Cannot reparent", "One or more entities you select are the ancestors of the destination");
				return ret;
			}
		}
		for (auto i = 0; i < es.n; i++)
			ret.push_back(es.p[i]);
	}
	else if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
	{
		auto path = Path::reverse(std::wstring((wchar_t*)payload->Data));
		auto ext = path.extension();
		if (ext == L".prefab")
		{
			if (get_root_prefab_instance(e_dst))
			{
				open_message_dialog("[RestructurePrefabInstanceWarnning]", "");
				return ret;
			}

			auto e = Entity::create(path);
			new PrefabInstance(e, path);
			ret.push_back(e);
		}
		else if (ext == L".timeline")
		{
			app.open_timeline(path);
			if (app.opened_timeline)
				app.set_timeline_host(e_dst);
		}
	}
	return ret;
}

bool entity_drag_behaviour(EntityPtr e)
{
	if (ImGui::BeginDragDropSource())
	{
		auto in_selection = false;
		if (selection.type == Selection::tEntity && !selection.lock)
		{
			auto found = false;
			auto es = selection.get_entities();
			for (auto _e : es)
			{
				if (e == _e)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				es.push_back(e);
				selection.select(es, "hierarchy"_h);
			}
			in_selection = true;
		}
		if (!in_selection || selection.objects.size() == 1)
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
		return true;
	}
	return false;
}

void entity_drop_behaviour(EntityPtr t)
{
	if (ImGui::BeginDragDropTarget())
	{
		auto es = read_drops(t);
		if (!es.empty())
		{
			for (auto _e : es)
			{
				if (_e->parent)
					_e->remove_from_parent(false);
			}
			for (auto _e : es)
			{
				t->add_child(_e);
				if (auto ins = get_root_prefab_instance(t); ins)
					ins->mark_modification(_e->parent->file_id.to_string() + (!_e->prefab_instance ? '|' + _e->file_id.to_string() : "") + "|add_child");
			}
			app.prefab_unsaved = true;
		}
		ImGui::EndDragDropTarget();
	}
}

void entity_context_menu_behaviour(EntityPtr e)
{
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
	{
		show_entities_menu();
		ImGui::EndPopup();
	}
}

void View_Hierarchy::on_draw()
{
	EntityPtr select_entity = nullptr;
	static bool released_after_select = false;
	EntityPtr focus_entity = selection_changed ? (selection.type == Selection::tEntity ? selection.as_entity(-1) : nullptr) : nullptr;
	static EntityPtr rename_entity = nullptr;

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

	auto content_pos = ImGui::GetCursorPos();

	std::function<void(EntityPtr)> show_entity;
	show_entity = [&](EntityPtr e) {
		auto root_ins = get_root_prefab_instance(e);
		auto is_added_to_ins = root_ins ? root_ins->find_modification(
			e->parent->file_id.to_string() + (!e->prefab_instance ? '|' + e->file_id.to_string() : "") + "|add_child") != -1 : false;

		std::string icon_string = is_added_to_ins ? "[+] " : "[ ] ";
		ImColor icon_color = (e->prefab_instance.get() == root_ins && root_ins) ? ImColor(127, 214, 252, 255) : (ImColor)ImGui::GetStyleColorVec4(ImGuiCol_Text);
		ImColor name_color = root_ins && (e->prefab_instance || !is_added_to_ins) ? ImColor(127, 214, 252, 255) : (ImColor)ImGui::GetStyleColorVec4(ImGuiCol_Text);

		auto selected = selection.selecting(e);

		auto flags = selected ? ImGuiTreeNodeFlags_Selected : 0;
		if (e != rename_entity)
			flags |= ImGuiTreeNodeFlags_SpanFullWidth;
		if (e->children.empty())
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		else
			flags |= ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
		if (!open_nodes.empty())
		{
			if (std::find(open_nodes.begin(), open_nodes.end(), e) != open_nodes.end())
				ImGui::SetNextItemOpen(true);
		}
		ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)icon_color);
		icon_string += "###";
		icon_string += str((uint64)e);
		auto indent = ImGui::GetCurrentWindow()->DC.Indent.x;
		const auto name_offset = 22.f;
		auto opened = ImGui::TreeNodeEx(icon_string.c_str(), flags);
		opened = opened && !(flags & ImGuiTreeNodeFlags_Leaf);
		if (rename_entity != e)
		{
			auto p0 = ImGui::GetItemRectMin();
			auto dl = ImGui::GetWindowDrawList();
			dl->AddText(ImVec2(p0.x + indent + name_offset, p0.y), name_color, e->name.c_str());
		}
		if (e == focus_entity)
			ImGui::SetScrollHereY();
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered())
		{
			if (selected)
			{
				if (released_after_select && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (rename_entity != e)
					{
						auto x = ImGui::GetMousePos().x;
						auto p0 = ImGui::GetItemRectMin();
						if (x > p0.x + indent + name_offset && x < p0.x + indent + name_offset + ImGui::CalcTextSize(e->name.c_str()).x)
						{
							rename_entity = e;
							rename_string = e->name;
							rename_start_frame = frames;
						}
					}
				}
			}
			else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				select_entity = e;
		}

		if (entity_drag_behaviour(e))
			released_after_select = false;

		if (rename_entity == e)
		{
			ImGui::SameLine();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushItemWidth(-1);
			if (frames == rename_start_frame)
				ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##rename", &rename_string, ImGuiInputTextFlags_AutoSelectAll))
			{
				e->name = rename_string;
				if (auto ins = get_root_prefab_instance(e); ins)
					ins->mark_modification(e->file_id.to_string() + "|name");
				if (!app.e_playing)
					app.prefab_unsaved = true;
			}
			if (frames != rename_start_frame)
			{
				if (ImGui::IsItemDeactivated() || (!ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
					rename_entity = nullptr;
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleVar();
		}

		entity_drop_behaviour(e);
		entity_context_menu_behaviour(e);

		if (opened)
		{
			auto gap_item = [&](int i) {
				ImGui::PushID(i);
				ImGui::InvisibleButton("gap", ImVec2(-1, 2));
				ImGui::PopID();
				if (ImGui::BeginDragDropTarget())
				{
					auto es = read_drops(e);
					if (!es.empty())
					{
						auto idx = i;
						for (auto _e : es)
						{
							if (_e->parent)
							{
								if (_e->parent == e && idx > _e->index)
									idx--;
								_e->remove_from_parent(false);
							}
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
				show_entity(e->children[i].get());
			}
			gap_item(i);
			ImGui::TreePop();
		}
	};

	if (auto root = app.e_playing ? app.e_playing : app.e_prefab; root)
	{
		auto w = ImGui::GetContentRegionAvail().x;
		auto filter_w = w - 20.f;
		if (!filter.empty())
		{
			auto cursor_pos = ImGui::GetCursorPos();
			ImGui::SetCursorPos(ImVec2(cursor_pos.x + filter_w - 19.f, cursor_pos.y + 3));
			if (ImGui::SmallButton(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
				filter.clear();
			ImGui::SetCursorPos(cursor_pos);
		}
		ImGui::SetNextItemWidth(filter_w);
		ImGui::InputText(graphics::FontAtlas::icon_s("magnifying-glass"_h).c_str(), &filter);

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
		if (filter.empty())
			show_entity(root);
		else
		{
			root->forward_traversal([&](EntityPtr e) {
				if (e->name.contains(filter))
				{
					auto in_prefab = get_root_prefab_instance(e);

					std::string display_name;
					if (e != rename_entity)
						display_name = e->name;
					if (e->prefab_instance)
						display_name = "[-] " + display_name;
					else
						display_name = "[ ] " + display_name;

					auto selected = selection.selecting(e);

					if (in_prefab)
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.8f, 1.f, 1.f));
					ImGui::Selectable(display_name.c_str(), selected);
					if (in_prefab)
						ImGui::PopStyleColor();
					if (ImGui::IsItemHovered())
					{
						if (selected)
						{

						}
						else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))
							select_entity = e;
					}

					if (entity_drag_behaviour(e))
						released_after_select = false;

					entity_drop_behaviour(e);
					entity_context_menu_behaviour(e);
				}
			});
		}
		ImGui::PopStyleVar(2);
	}

	ImGui::SetCursorPos(content_pos);
	ImGui::InvisibleButton("##background", ImGui::GetContentRegionAvail());
	if (ImGui::IsItemClicked())
	{
		released_after_select = false;
		selection.clear("hierarchy"_h);
	}
	else if (select_entity)
	{
		released_after_select = false;
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
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		released_after_select = true;

	auto& io = ImGui::GetIO();

	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered())
	{
		if (!io.WantCaptureKeyboard)
		{
			if (ImGui::IsKeyPressed(Keyboard_Del))
				app.cmd_delete_entities(selection.get_entities());
			if (ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_D))
				app.cmd_duplicate_entities(selection.get_entities());
		}
	}

	selection_changed = false;
}
