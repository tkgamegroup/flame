#include "selection.h"
#include "history.h"
#include "hierarchy_window.h"
#include "scene_window.h"

HierarchyWindow hierarchy_window;

HierarchyView::HierarchyView() :
	HierarchyView(hierarchy_window.views.empty() ? "Hierarchy" : "Hierarchy##" + str(rand()))
{
}

HierarchyView::HierarchyView(const std::string& name) :
	View(&hierarchy_window, name)
{
}

static auto selection_changed = false;

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
		if (selection.type == Selection::tEntity)
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
			std::vector<GUID> old_parents;
			std::vector<uint> old_indices;

			for (auto _e : es)
			{
				if (_e->parent)
				{
					if (!app.e_playing && app.e_prefab)
					{
						old_parents.push_back(_e->parent->instance_id);
						old_indices.push_back(_e->index);
					}
					if (auto ins = get_root_prefab_instance(_e); ins)
						ins->remove_modification(_e->parent->file_id.to_string() + (!_e->prefab_instance ? '|' + _e->file_id.to_string() : "") + "|add_child");
					_e->remove_from_parent(false);
				}
			}
			for (auto _e : es)
			{
				t->add_child(_e);
				if (auto ins = get_root_prefab_instance(t); ins)
					ins->mark_modification(_e->parent->file_id.to_string() + (!_e->prefab_instance ? '|' + _e->file_id.to_string() : "") + "|add_child");
			}

			if (!app.e_playing && app.e_prefab)
			{
				std::vector<GUID> ids(es.size());
				std::vector<GUID> new_parents(es.size());
				std::vector<uint> new_indices(es.size());
				for (auto i = 0; i < es.size(); i++)
				{
					ids[i] = es[i]->instance_id;
					new_parents[i] = es[i]->parent->instance_id;
					new_indices[i] = es[i]->index;
				}
				auto h = new EntityPositionHistory(ids, old_parents, old_indices, new_parents, new_indices);
				add_history(h);
				if (h->ids.size() == 1)
					app.last_status = std::format("Entity Reparented: {}", es[0]->name);
				else
					app.last_status = std::format("{} Entities Reparented", (int)h->ids.size());

				app.prefab_unsaved = true;
			}
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

void HierarchyView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened, app.prefab_unsaved ? ImGuiWindowFlags_UnsavedDocument : 0);
	imgui_window = ImGui::GetCurrentWindow();

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
		auto name_offset = ImGui::CalcTextSize(icon_string.c_str()).x;
		icon_string += "###";
		icon_string += str((uint64)e);
		auto indent = ImGui::GetCurrentWindow()->DC.Indent.x;
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
						std::vector<GUID> old_parents;
						std::vector<uint> old_indices;

						auto t = e;
						auto idx = i;
						for (auto _e : es)
						{
							if (_e->parent)
							{
								if (!app.e_playing && app.e_prefab)
								{
									old_parents.push_back(_e->parent->instance_id);
									old_indices.push_back(_e->index);
								}
								if (_e->parent == t && idx > _e->index)
									idx--;
								if (auto ins = get_root_prefab_instance(_e); ins)
									ins->remove_modification(_e->parent->file_id.to_string() + (!_e->prefab_instance ? '|' + _e->file_id.to_string() : "") + "|add_child");
								_e->remove_from_parent(false);
							}
						}
						for (auto _e : es)
						{
							t->add_child(_e, idx);
							if (auto ins = get_root_prefab_instance(t); ins)
								ins->mark_modification(_e->parent->file_id.to_string() + (!_e->prefab_instance ? '|' + _e->file_id.to_string() : "") + "|add_child");
							idx++;
						}

						if (!app.e_playing && app.e_prefab)
						{
							std::vector<GUID> ids(es.size());
							std::vector<GUID> new_parents(es.size());
							std::vector<uint> new_indices(es.size());
							for (auto i = 0; i < es.size(); i++)
							{
								ids[i] = es[i]->instance_id;
								new_parents[i] = es[i]->parent->instance_id;
								new_indices[i] = es[i]->index;
							}
							auto h = new EntityPositionHistory(ids, old_parents, old_indices, new_parents, new_indices);
							add_history(h);
							if (h->ids.size() == 1)
								app.last_status = std::format("Entity Repositioned: {}", es[0]->name);
							else
								app.last_status = std::format("{} Entities Repositioned", (int)h->ids.size());

							app.prefab_unsaved = true;
						}
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
			if (ImGui::SmallButton(graphics::font_icon_str("xmark"_h).c_str()))
				filter.clear();
			ImGui::SetCursorPos(cursor_pos);
		}
		ImGui::SetNextItemWidth(filter_w);
		ImGui::InputText(graphics::font_icon_str("magnifying-glass"_h).c_str(), &filter);

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
	ImGui::InvisibleButton("##background", max(vec2(1.f), (vec2)ImGui::GetContentRegionAvail()));
	if (select_entity)
	{
		released_after_select = false;
		if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl))
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
		else if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Shift))
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
						auto id = window->GetID(("###" + str((uint64)e)).c_str());
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
	else if (ImGui::IsItemClicked())
	{
		released_after_select = false;
		selection.clear("hierarchy"_h);
	}
	if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		released_after_select = true;

	auto& io = ImGui::GetIO();

	if (ImGui::IsWindowHovered())
	{
		if (!io.WantCaptureKeyboard)
		{
			if (ImGui::IsKeyPressed((ImGuiKey)Keyboard_Del))
				app.cmd_delete_entities(selection.get_entities());
			if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Shift) && ImGui::IsKeyPressed((ImGuiKey)Keyboard_D))
				app.cmd_duplicate_entities(selection.get_entities());
			if (ImGui::IsKeyDown((ImGuiKey)Keyboard_Ctrl) && ImGui::IsKeyPressed((ImGuiKey)Keyboard_S))
				app.save_prefab();
		}
	}

	selection_changed = false;

	ImGui::End();
	if (!opened)
		delete this;
}

HierarchyWindow::HierarchyWindow() :
	Window("Hierarchy")
{
}

void HierarchyWindow::init()
{
	selection.callbacks.add([](uint caller) {
		if (caller != "hierarchy"_h)
			selection_changed = true;
	}, "hierarchy"_h);
}

View* HierarchyWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new HierarchyView;
	return nullptr;
}

View* HierarchyWindow::open_view(const std::string& name)
{
	return new HierarchyView(name);
}
