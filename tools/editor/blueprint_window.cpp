#include "blueprint_window.h"
#include "project_window.h"

#include <flame/foundation/blueprint.h>
#include <flame/graphics/model.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>

struct BpObjectT
{
	virtual ~BpObjectT() {}

	uint object_id;
};

BlueprintObject get_object_from_ax_node_id(ax::NodeEditor::NodeId id, BlueprintInstance::Group& g)
{
	auto obj = (BpObjectT*)(uint64)id;
	return g.object_map[obj->object_id]->original;
}

BlueprintWindow blueprint_window;

BlueprintView::BlueprintView() :
	BlueprintView(blueprint_window.views.empty() ? "Blueprint" : "Blueprint##" + str(rand()))
{
}

BlueprintView::BlueprintView(const std::string& name) :
	View(&blueprint_window, name)
{
	auto sp = SUS::split(name, '#');
	if (sp.size() > 1)
		blueprint_path = sp[0];

#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::Config ax_node_editor_config;
	ax_node_editor_config.UserPointer = this;
	ax_node_editor_config.SettingsFile = "";
	ax_node_editor_config.NavigateButtonIndex = 2;
	ax_node_editor_config.SaveNodeSettings = [](ax::NodeEditor::NodeId node_id, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* user_data) {
		auto& view = *(BlueprintView*)user_data;
		if (frames == view.load_frame)
			return true;
		auto& instance_group = view.blueprint_instance->groups[view.group_name_hash];
		auto obj = get_object_from_ax_node_id(node_id, instance_group);
		if (ImGui::IsKeyDown(Keyboard_Alt))
			view.expand_block_sizes();
		view.process_object_moved(obj);
		view.unsaved = true;
		return true;
	};
	ax_node_editor = (ax::NodeEditor::Detail::EditorContext*)ax::NodeEditor::CreateEditor(&ax_node_editor_config);
#endif
}

struct BpNodePreview
{
	ModelPreviewer model_previewer;

	~BpNodePreview()
	{
		model_previewer.destroy();
	}
};
static std::map<BlueprintNodePtr, BpNodePreview> previews;

BlueprintView::~BlueprintView()
{
	if (app_exiting)
		return;
	if (blueprint)
	{
		previews.clear();
		Blueprint::release(blueprint);
	}
	if (blueprint_instance)
	{
		delete blueprint_instance;
	}
	if (ax_node_editor)
	{
		ax::NodeEditor::DestroyEditor((ax::NodeEditor::EditorContext*)ax_node_editor);
	}
}

void BlueprintView::process_object_moved(BlueprintObject obj)
{
	auto g = blueprint->find_group(group_name_hash);
	auto remove_invalid_links = [&](BlueprintNodePtr node) {
		std::vector<BlueprintLinkPtr> to_remove_links;
		for (auto& l : g->links)
		{
			if (l->to_slot->parent.p.node == node)
			{
				if (l->from_slot->parent.get_depth() > l->to_slot->parent.get_depth())
					to_remove_links.push_back(l.get());
			}
		}
		for (auto l : to_remove_links)
			blueprint->remove_link(l);
	};
	std::function<void(BlueprintBlockPtr)> remove_block_invalid_links;
	remove_block_invalid_links = [&](BlueprintBlockPtr block) {
		for (auto c : block->children)
		{
			remove_block_invalid_links(c);
			for (auto& l : g->links)
			{
				if (l->to_slot == c->input.get())
				{
					if (l->from_slot->parent.get_depth() > l->to_slot->parent.get_depth())
						blueprint->remove_link(l.get());
					break;
				}
			}
		}
		for (auto n : block->nodes)
			remove_invalid_links(n);
	};
	auto try_change_node_block = [&](BlueprintNodePtr node) {
		Rect node_rect;
		node_rect.a = node->position;
		node_rect.b = node_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)node);
		BlueprintBlockPtr most_depth_block = nullptr;
		uint most_depth = 0;
		auto g = node->group;
		for (auto& b : g->blocks)
		{
			if (b->rect.contains(node_rect))
			{
				if (b->depth > most_depth)
				{
					most_depth_block = b.get();
					most_depth = b->depth;
				}
			}
		}

		auto new_block = most_depth_block ? most_depth_block : g->blocks.front().get();
		if (node->block != new_block)
			blueprint->set_node_block(node, new_block);
	};

	switch (obj.type)
	{
	case BlueprintObjectNode:
		try_change_node_block(obj.p.node);
		break;
	case BlueprintObjectBlock:
	{
		for (auto& b : g->blocks)
		{
			if (b->depth == 0) // skip the root block
				continue;

			Rect block_rect;
			block_rect.a = b->position;
			block_rect.b = block_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)b.get());

			BlueprintBlockPtr most_depth_parent = nullptr;
			uint most_depth = 0;
			for (auto& b2 : g->blocks)
			{
				if (b2->depth == 0) // skip the root block
					continue;

				if (b2->rect.contains(block_rect))
				{
					if (b2->depth > most_depth)
					{
						most_depth_parent = b2.get();
						most_depth = b2->depth;
					}
				}
			}

			auto new_parent = most_depth_parent ? most_depth_parent : g->blocks.front().get();
			if (b->parent != new_parent)
				blueprint->set_block_parent(b.get(), new_parent);
		}
		for (auto& n : g->nodes)
			try_change_node_block(n.get());
	}
		break;
	}

	remove_block_invalid_links(g->blocks.front().get());
}

void BlueprintView::expand_block_sizes()
{
	auto g = blueprint->find_group(group_name_hash);
	std::function<void(BlueprintBlockPtr)> fit_block_size;
	fit_block_size = [&](BlueprintBlockPtr b) {
		Rect rect = b->rect;

		for (auto& c : b->children)
		{
			fit_block_size(c);

			Rect block_rect;
			block_rect.a = c->position;
			block_rect.b = block_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)c);
			block_rect.expand(10.f);
			rect.expand(block_rect);
		}

		for (auto& n : b->nodes)
		{
			Rect node_rect;
			node_rect.a = n->position;
			node_rect.b = node_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)n);
			node_rect.expand(10.f);
			rect.expand(node_rect);
		}

		if (b != g->blocks.front().get())
		{
			if (any(lessThan(rect.a, b->rect.a)) || any(greaterThan(rect.b, b->rect.b)))
			{
				b->position -= (b->rect.a - rect.a);
				b->rect = rect;
				ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
				auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)b);
				ax_node->m_GroupBounds.Min = b->rect.a;
				ax_node->m_GroupBounds.Max = b->rect.b;

			}
		}
	};
	fit_block_size(g->blocks.front().get());
}

void BlueprintView::load_blueprint(const std::filesystem::path& path)
{
	assert(!blueprint && !blueprint_instance);
	blueprint = Blueprint::get(path);
	blueprint_instance = BlueprintInstance::create(blueprint);
}

void BlueprintView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened, unsaved ? ImGuiWindowFlags_UnsavedDocument : 0);

	ax::NodeEditor::SetCurrentEditor((ax::NodeEditor::EditorContext*)ax_node_editor);

	auto frame = frames;
	if (!blueprint)
	{
		load_blueprint(blueprint_path);
		load_frame = frame;
	}
	if (blueprint)
	{
		if (blueprint_instance->built_frame < blueprint->dirty_frame)
			blueprint_instance->build();

		if (ImGui::Button("Save"))
		{
			if (unsaved)
			{
				blueprint->save();
				unsaved = false;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Zoom To Content"))
		{
			ax::NodeEditor::NavigateToContent(0.f);
			//app.render_frames += 24;
		}

		if (ImGui::BeginTable("bp_editor", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("side_panel", ImVec2(0, -2));

			auto ti_str = [&](TypeInfo* ti) {
				return TypeInfo::serialize_t(ti->tag) + '@' + ti->name;
			};

			static std::string type_filter = "";
			auto show_types_menu = [&]() {
				TypeInfo* ret = nullptr;

				ImGui::InputText("Filter", &type_filter);
				if (ImGui::BeginMenu("Enum"))
				{
					for (auto& ei : tidb.enums)
					{
						if (!type_filter.empty())
						{
							if (ei.second.name.find(type_filter) == std::string::npos)
								continue;
						}
						if (ImGui::Selectable(ei.second.name.c_str()))
							ret = TypeInfo::get(TagE, ei.second.name, tidb);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Data"))
				{
					for (auto bt : tidb.basic_types)
					{
						if (!type_filter.empty())
						{
							if (bt->name.find(type_filter) == std::string::npos)
								continue;
						}
						if (ImGui::Selectable(bt->name.c_str()))
							ret = TypeInfo::get(TagD, bt->name, tidb);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("UDT"))
				{
					for (auto& ui : tidb.udts)
					{
						if (!type_filter.empty())
						{
							if (ui.second.name.find(type_filter) == std::string::npos)
								continue;
						}
						if (ImGui::Selectable(ui.second.name.c_str()))
							ret = TypeInfo::get(TagU, ui.second.name, tidb);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Pointer"))
				{
					if (ImGui::BeginMenu("Of Enum"))
					{
						for (auto& ei : tidb.enums)
						{
							if (!type_filter.empty())
							{
								if (ei.second.name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(ei.second.name.c_str()))
								ret = TypeInfo::get(TagPE, ei.second.name, tidb);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Of Data"))
					{
						for (auto bt : tidb.basic_types)
						{
							if (!type_filter.empty())
							{
								if (bt->name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(bt->name.c_str()))
								ret = TypeInfo::get(TagPD, bt->name, tidb);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Of Udt"))
					{
						for (auto& ui : tidb.udts)
						{
							if (!type_filter.empty())
							{
								if (ui.second.name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(ui.second.name.c_str()))
								ret = TypeInfo::get(TagPU, ui.second.name, tidb);
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Vector"))
				{
					if (ImGui::BeginMenu("Of Enum"))
					{
						for (auto& ei : tidb.enums)
						{
							if (!type_filter.empty())
							{
								if (ei.second.name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(ei.second.name.c_str()))
								ret = TypeInfo::get(TagVE, ei.second.name, tidb);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Of Data"))
					{
						for (auto bt : tidb.basic_types)
						{
							if (!type_filter.empty())
							{
								if (bt->name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(bt->name.c_str()))
								ret = TypeInfo::get(TagVD, bt->name, tidb);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Of Udt"))
					{
						for (auto& ui : tidb.udts)
						{
							if (!type_filter.empty())
							{
								if (ui.second.name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(ui.second.name.c_str()))
								ret = TypeInfo::get(TagVU, ui.second.name, tidb);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Of Pointer Of Udt"))
					{
						for (auto& ui : tidb.udts)
						{
							if (!type_filter.empty())
							{
								if (ui.second.name.find(type_filter) == std::string::npos)
									continue;
							}
							if (ImGui::Selectable(ui.second.name.c_str()))
								ret = TypeInfo::get(TagVPU, ui.second.name, tidb);
						}
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				return ret;
			};

			auto manipulate_value = [](TypeInfo* type, void* data) {
				auto changed = false;
				if (type->tag == TagD)
				{
					auto ti = (TypeInfo_Data*)type;
					switch (ti->data_type)
					{
					case DataBool:
						ImGui::SetNextItemWidth(100.f);
						changed |= ImGui::Checkbox("", (bool*)data);
						break;
					case DataFloat:
						ImGui::PushMultiItemsWidths(ti->vec_size, 60.f * ti->vec_size);
						for (int i = 0; i < ti->vec_size; i++)
						{
							ImGui::PushID(i);
							if (i > 0)
								ImGui::SameLine();
							ImGui::DragScalar("", ImGuiDataType_Float, &((float*)data)[i], 0.01f);
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							ImGui::PopID();
							ImGui::PopItemWidth();
						}
						break;
					case DataInt:
						ImGui::PushMultiItemsWidths(ti->vec_size, 60.f * ti->vec_size);
						for (int i = 0; i < ti->vec_size; i++)
						{
							ImGui::PushID(i);
							if (i > 0)
								ImGui::SameLine();
							ImGui::DragScalar("", ImGuiDataType_S32, &((int*)data)[i]);
							changed |= ImGui::IsItemDeactivatedAfterEdit();
							ImGui::PopID();
							ImGui::PopItemWidth();
						}
						break;
					case DataChar:
						if (ti->vec_size == 4)
						{
							vec4 color = *(cvec4*)data;
							color /= 255.f;
							ImGui::SetNextItemWidth(160.f);
							changed |= ImGui::ColorEdit4("", &color[0]);
							if (changed)
								*(cvec4*)data = color * 255.f;
						}
						break;
					case DataString:
						ImGui::SetNextItemWidth(100.f);
						ImGui::InputText("", (std::string*)data);
						changed |= ImGui::IsItemDeactivatedAfterEdit();
						break;
					case DataWString:
					{
						auto s = w2s(*(std::wstring*)data);
						ImGui::SetNextItemWidth(100.f);
						ImGui::InputText("", &s);
						changed |= ImGui::IsItemDeactivatedAfterEdit();
						if (changed)
							*(std::wstring*)data = s2w(s);
					}
						break;
					case DataPath:
					{
						auto& path = *(std::filesystem::path*)data;
						auto s = path.string();
						ImGui::SetNextItemWidth(100.f);
						ImGui::InputText("", s.data(), ImGuiInputTextFlags_ReadOnly);
						if (ImGui::BeginDragDropTarget())
						{
							if (auto payload = ImGui::AcceptDragDropPayload("File"); payload)
							{
								path = Path::reverse(std::wstring((wchar_t*)payload->Data));
								changed = true;
							}
							ImGui::EndDragDropTarget();
						}
						ImGui::SameLine();
						if (ImGui::Button(graphics::font_icon_str("location-crosshairs"_h).c_str()))
							project_window.ping(Path::get(path));
						ImGui::SameLine();
						if (ImGui::Button(graphics::font_icon_str("xmark"_h).c_str()))
						{
							path = L"";
							changed = true;
						}
					}
						break;
					}
				}
				return changed;
			};

			if (ImGui::CollapsingHeader("Bp Variables:"))
			{
				ImGui::PushID("bp_variables");
				static int selected_variable = -1;
				ImGui::SetNextItemWidth(150.f);
				if (ImGui::BeginListBox("##variables"))
				{
					for (auto i = 0; i < blueprint->variables.size(); i++)
					{
						if (ImGui::Selectable(blueprint->variables[i].name.c_str(), selected_variable == i))
							selected_variable = i;
					}
					ImGui::EndListBox();
				}
				selected_variable = min(selected_variable, (int)blueprint->variables.size() - 1);
				ImGui::SameLine();
				ImGui::BeginGroup();
				if (selected_variable != -1)
				{
					auto& var = blueprint->variables[selected_variable];

					ImGui::SetNextItemWidth(200.f);
					ImGui::InputText("Name", &var.name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						auto name = var.name;
						auto type = var.type;
						blueprint->remove_variable(nullptr, var.name_hash);
						blueprint->add_variable(nullptr, name, type);
						selected_variable = -1;
						unsaved = true;
					}
					ImGui::SetNextItemWidth(200.f);
					if (ImGui::BeginCombo("Type", ti_str(var.type).c_str()))
					{
						auto type = show_types_menu();
						if (type)
						{
							auto name = var.name;
							blueprint->remove_variable(nullptr, var.name_hash);
							blueprint->add_variable(nullptr, name, type);
							selected_variable = -1;
							unsaved = true;
						}

						ImGui::EndCombo();
					}
					ImGui::TextUnformatted("Value");
					auto changed = manipulate_value(var.type, var.data);
					if (changed)
						unsaved = true;
				}
				ImGui::EndGroup();

				if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
				{
					auto name = get_unique_name("new_variable", [&](const std::string& name) {
						for (auto& v : blueprint->variables)
						{
							if (v.name == name)
								return true;
						}
						return false;
					});
					blueprint->add_variable(nullptr, name, TypeInfo::get<float>());
					selected_variable = blueprint->variables.size() - 1;

				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
				{
					if (selected_variable != -1)
						blueprint->remove_variable(nullptr, blueprint->variables[selected_variable].name_hash);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_variable > 0)
						std::swap(blueprint->variables[selected_variable], blueprint->variables[selected_variable - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_variable != -1 && selected_variable < blueprint->variables.size() - 1)
						std::swap(blueprint->variables[selected_variable], blueprint->variables[selected_variable + 1]);
				}
				ImGui::PopID();

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}

			ImGui::Separator();

			auto group = blueprint->find_group(group_name_hash);
			if (!group)
			{
				group = blueprint->groups[0].get();
				group_name = group->name;
				group_name_hash = group->name_hash;
			}

			if (ImGui::BeginCombo("##group_dropdown", "", ImGuiComboFlags_NoPreview))
			{
				for (auto& g : blueprint->groups)
				{
					if (ImGui::Selectable(g->name.c_str()))
					{
						group = g.get();
						group_name = group->name;
						group_name_hash = group->name_hash;
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(100.f);
			ImGui::InputText("##group", &group_name);
			if (ImGui::IsItemDeactivatedAfterEdit())
			{
				group_name_hash = sh(group_name.c_str());
				group->name = group_name;
				group->name_hash = group_name_hash;
				group->structure_changed_frame = frame;
				blueprint->dirty_frame = frame;
			}
			ImGui::SameLine();

			if (ImGui::Button(graphics::font_icon_str("xmark"_h).c_str()))
			{
				blueprint->remove_group(group);
				group = blueprint->groups.back().get();
				group_name = group->name;
				group_name_hash = group->name_hash;

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();

			}
			ImGui::SameLine();
			if (ImGui::Button("New Group"))
			{
				auto name = get_unique_name("new_group", [&](const std::string& name) {
					for (auto& g : blueprint->groups)
					{
						if (g->name == name)
							return true;
					}
					return false;
				});
				group = blueprint->add_group(name);
				group_name = group->name;
				group_name_hash = group->name_hash;

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}

			auto debugging_instance = blueprint_window.debugger->debugging &&
				blueprint_window.debugger->debugging->blueprint == blueprint ?
				blueprint_window.debugger->debugging : nullptr;
			auto& instance_group = debugging_instance ? debugging_instance->groups[group_name_hash] : blueprint_instance->groups[group_name_hash];

			if (ImGui::CollapsingHeader("Group Variables:"))
			{
				ImGui::PushID("group_variables");
				static int selected_variable = -1;
				ImGui::SetNextItemWidth(150.f);
				if (ImGui::BeginListBox("##variables"))
				{
					for (auto i = 0; i < group->variables.size(); i++)
					{
						if (ImGui::Selectable(group->variables[i].name.c_str(), selected_variable == i))
							selected_variable = i;
					}
					ImGui::EndListBox();
				}
				selected_variable = min(selected_variable, (int)group->variables.size() - 1);
				ImGui::SameLine();
				ImGui::BeginGroup();
				if (selected_variable != -1)
				{
					auto& var = group->variables[selected_variable];

					ImGui::SetNextItemWidth(200.f);
					ImGui::InputText("Name", &var.name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						auto name = var.name;
						auto type = var.type;
						blueprint->remove_variable(group, var.name_hash);
						blueprint->add_variable(group, name, type);
						selected_variable = -1;
						unsaved = true;
					}
					ImGui::SetNextItemWidth(200.f);
					if (ImGui::BeginCombo("Type", ti_str(var.type).c_str()))
					{
						auto type = show_types_menu();
						if (type)
						{
							auto name = var.name;
							blueprint->remove_variable(group, var.name_hash);
							blueprint->add_variable(group, name, type);
							selected_variable = -1;
							unsaved = true;
						}

						ImGui::EndCombo();
					}
					ImGui::TextUnformatted("Value");
					auto changed = manipulate_value(var.type, var.data);
					if (changed)
						unsaved = true;
				}
				ImGui::EndGroup();

				if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
				{
					auto name = get_unique_name("new_variable", [&](const std::string& name) {
						for (auto& v : group->variables)
						{
							if (v.name == name)
								return true;
						}
						return false;
					});
					blueprint->add_variable(group, name, TypeInfo::get<float>());
					selected_variable = group->variables.size() - 1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
				{
					if (selected_variable != -1)
						blueprint->remove_variable(group, group->variables[selected_variable].name_hash);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_variable > 0)
						std::swap(group->variables[selected_variable], group->variables[selected_variable - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_variable != -1 && selected_variable < group->variables.size() - 1)
						std::swap(group->variables[selected_variable], group->variables[selected_variable + 1]);
				}
				ImGui::PopID();

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}

			if (ImGui::CollapsingHeader("Group Inputs:"))
			{
				ImGui::PushID("group_inputs");
				static int selected_input = -1;
				ImGui::SetNextItemWidth(150.f);
				if (ImGui::BeginListBox("##inputs"))
				{
					for (auto i = 0; i < group->inputs.size(); i++)
					{
						if (ImGui::Selectable(group->inputs[i].name.c_str(), selected_input == i))
							selected_input = i;
					}
					ImGui::EndListBox();
				}
				selected_input = min(selected_input, (int)group->inputs.size() - 1);
				ImGui::SameLine();
				ImGui::BeginGroup();
				if (selected_input != -1)
				{
					auto& var = group->inputs[selected_input];

					ImGui::SetNextItemWidth(200.f);
					ImGui::InputText("Name", &var.name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						auto name = var.name;
						auto type = var.type;
						blueprint->remove_group_input(group, var.name_hash);
						blueprint->add_group_input(group, name, type);
						selected_input = -1;
					}
					ImGui::SetNextItemWidth(200.f);
					if (ImGui::BeginCombo("Type", ti_str(var.type).c_str()))
					{
						auto type = show_types_menu();
						if (type)
						{
							auto name = var.name;
							blueprint->remove_group_input(group, var.name_hash);
							blueprint->add_group_input(group, name, type);
							selected_input = -1;
						}

						ImGui::EndCombo();
					}
				}
				ImGui::EndGroup();

				if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
				{
					auto name = get_unique_name("new_input", [&](const std::string& name) {
						for (auto& i : group->inputs)
						{
							if (i.name == name)
								return true;
						}
						return false;
					});
					blueprint->add_group_input(group, name, TypeInfo::get<float>());
					selected_input = group->inputs.size() - 1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
				{
					if (selected_input != -1)
						blueprint->remove_group_input(group, group->inputs[selected_input].name_hash);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_input > 0)
						std::swap(group->inputs[selected_input], group->inputs[selected_input - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_input != -1 && selected_input < group->inputs.size() - 1)
						std::swap(group->inputs[selected_input], group->inputs[selected_input + 1]);
				}
				ImGui::PopID();

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}
			if (ImGui::CollapsingHeader("Group Outputs:"))
			{
				ImGui::PushID("group_outputs");
				static int selected_output = -1;
				ImGui::SetNextItemWidth(150.f);
				if (ImGui::BeginListBox("##outputs"))
				{
					for (auto i = 0; i < group->outputs.size(); i++)
					{
						if (ImGui::Selectable(group->outputs[i].name.c_str(), selected_output == i))
							selected_output = i;
					}
					ImGui::EndListBox();
				}
				selected_output = min(selected_output, (int)group->outputs.size() - 1);
				ImGui::SameLine();
				ImGui::BeginGroup();
				if (selected_output != -1)
				{
					auto& var = group->outputs[selected_output];

					ImGui::SetNextItemWidth(200.f);
					ImGui::InputText("Name", &var.name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						auto name = var.name;
						auto type = var.type;
						blueprint->remove_group_output(group, var.name_hash);
						blueprint->add_group_output(group, name, type);
						selected_output = -1;
					}
					ImGui::SetNextItemWidth(200.f);
					if (ImGui::BeginCombo("Type", ti_str(var.type).c_str()))
					{
						auto type = show_types_menu();
						if (type)
						{
							auto name = var.name;
							blueprint->remove_group_output(group, var.name_hash);
							blueprint->add_group_output(group, name, type);
							selected_output = -1;
						}

						ImGui::EndCombo();
					}
				}
				ImGui::EndGroup();

				if (ImGui::SmallButton(graphics::font_icon_str("plus"_h).c_str()))
				{
					auto name = get_unique_name("new_output", [&](const std::string& name) {
						for (auto& o : group->outputs)
						{
							if (o.name == name)
								return true;
						}
						return false;
					});
					blueprint->add_group_output(group, name, TypeInfo::get<float>());
					selected_output = group->outputs.size() - 1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("minus"_h).c_str()))
				{
					if (selected_output != -1)
						blueprint->remove_group_output(group, group->outputs[selected_output].name_hash);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_output > 0)
						std::swap(group->outputs[selected_output], group->outputs[selected_output - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_output != -1 && selected_output < group->outputs.size() - 1)
						std::swap(group->outputs[selected_output], group->outputs[selected_output + 1]);
				}
				ImGui::PopID();

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}
			if (blueprint_window.debugger->debugging == blueprint_instance)
			{
				if (ImGui::CollapsingHeader("Group Slot Datas:"))
				{
					for (auto& d : instance_group.slot_datas)
					{
						auto& attr = d.second.attribute;
						ImGui::TextUnformatted(std::format("ID: {}, Type: {}, Value: {}",
							d.first, ti_str(attr.type), attr.type->serialize(attr.data)).c_str());
					}
				}
			}
			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);
			ImGui::BeginChild("main_area", ImVec2(0, -2));
			{
				static BlueprintGroupPtr last_group = nullptr;
				if (group != last_group)
				{
					for (auto& b : group->blocks)
					{
						if (b.get() != group->blocks.front().get())
						{
							ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b.get(), b->position);
							ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b.get(), b->rect.size());
						}
					}
					for (auto& n : group->nodes)
						ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n.get(), n->position);
					last_group = group;
					load_frame = frame;
				}

				auto step = [&]() {
					blueprint_window.debugger->debugging = nullptr;

					BlueprintNodePtr break_node = nullptr;
					if (auto o = debugging_instance->executing_object(); o && o->original.type == BlueprintObjectNode)
					{
						if (blueprint_window.debugger->has_break_node(o->original.p.node))
						{
							break_node = o->original.p.node;
							blueprint_window.debugger->remove_break_node(break_node);
						}
					}
					debugging_instance->step();
					if (break_node)
						blueprint_window.debugger->add_break_node(break_node);
				};

				if (ImGui::Button("Run"))
				{
					if (!debugging_instance)
					{
						blueprint_instance->prepare_executing(blueprint_instance->get_group(group_name_hash));
						blueprint_instance->run();
					}
					else
					{
						step();
						debugging_instance->run();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Step"))
				{
					if (!debugging_instance)
					{
						blueprint_instance->prepare_executing(blueprint_instance->get_group(group_name_hash));
						blueprint_window.debugger->debugging = blueprint_instance;
					}
					else
					{
						step();
						if (!debugging_instance->executing_stack.empty())
							blueprint_window.debugger->debugging = debugging_instance;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
				{
					if (blueprint_window.debugger->debugging == blueprint_instance)
					{
						blueprint_window.debugger->debugging = nullptr;
						blueprint_instance->stop();
					}
				}

				auto& io = ImGui::GetIO();
				auto dl = ImGui::GetWindowDrawList();
				std::string tooltip; vec2 tooltip_pos;
				auto get_slot_value = [](const BlueprintAttribute& arg)->std::string {
					if (arg.type->tag != TagD)
						return "";
					return std::format("Value: {}", arg.type->serialize(arg.data));
				};

				if (debugging_instance)
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImColor(100, 80, 60, 200));
				ax::NodeEditor::Begin("node_editor");

				auto executing_object = debugging_instance ? debugging_instance->executing_object() : blueprint_instance->executing_object();
					
				for (auto& b : group->blocks)
				{
					if (b->depth == 0) // skip root block
						continue;

					ax::NodeEditor::BeginNode((uint64)b.get());
					ImGui::Text("D%d", b->depth);

					b->position = ax::NodeEditor::GetNodePosition((ax::NodeEditor::NodeId)b.get());
					auto ax_node = ax_node_editor->GetNodeBuilder().m_CurrentNode;
					{
						auto bounds = ax_node->m_GroupBounds;
						b->rect.a = bounds.Min;
						b->rect.b = bounds.Max;
					}

					ax::NodeEditor::BeginPin((uint64)b->input.get(), ax::NodeEditor::PinKind::Input);
					ImGui::TextUnformatted((graphics::font_icon_str("play"_h) + "  ").c_str());
					ax::NodeEditor::EndPin();

					ImGui::SameLine(0.f, max(0.f, b->rect.size().x - 56.f));

					ax::NodeEditor::BeginPin((uint64)b->output.get(), ax::NodeEditor::PinKind::Output);
					ImGui::TextUnformatted(("  " + graphics::font_icon_str("play"_h)).c_str());
					ax::NodeEditor::EndPin();

					ax::NodeEditor::Group(b->rect.size());

					// restore last pin
					auto last_pin = ax_node->m_LastPin;

					ax::NodeEditor::EndNode();

					// recover last pin and re-active all pins, since our groups(blocks) can have pins
					ax_node->m_LastPin = last_pin;
					for (auto pin = ax_node->m_LastPin; pin; pin = pin->m_PreviousPin)
						pin->m_IsLive = true;
				}
				for (auto& n : group->nodes)
				{
					auto instance_object = instance_group.object_map[n->object_id];

					auto color = ax::NodeEditor::GetStyle().Colors[ax::NodeEditor::StyleColor_NodeBg];
					if (blueprint_window.debugger->has_break_node(n.get()))
					{
						if (executing_object && executing_object->original.p.node == n.get())
							color = ImColor(204, 116, 45, 200);
						else
							color = ImColor(197, 81, 89, 200);
					}
					else if (executing_object && executing_object->original.p.node == n.get())
						color = ImColor(211, 151, 0, 200);
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, color);

					ax::NodeEditor::BeginNode((uint64)n.get());
					auto display_name = n->display_name.empty() ? n->name : n->display_name;
					ImGui::TextUnformatted(display_name.c_str());
					ImGui::Text("D%d", n->block->depth + 1);
					ImGui::BeginGroup();
					for (auto i = 0; i < n->inputs.size(); i++)
					{
						auto input = n->inputs[i].get();
						if (input->flags & BlueprintSlotFlagHideInUI)
							continue;
						ax::NodeEditor::BeginPin((uint64)input, ax::NodeEditor::PinKind::Input);
						ImGui::Text("%s %s", graphics::font_icon_str("play"_h).c_str(), input->name_hash == "Execute"_h ? "" : input->name.c_str());
						ax::NodeEditor::EndPin();
						if (debugging_instance)
						{
							if (ImGui::IsItemHovered())
							{
								auto& arg = instance_object->inputs[i];
								if (arg.type)
								{
									tooltip = std::format("{} ({})\nObject ID: {}", get_slot_value(arg), ti_str(arg.type), input->object_id);
									ax::NodeEditor::Suspend();
									tooltip_pos = io.MousePos;
									ax::NodeEditor::Resume();
								}
							}
						}
						else
						{
							if (ImGui::IsItemHovered())
							{
								tooltip = std::format("({})\nObject ID: {}", ti_str(input->type), input->object_id);
								ax::NodeEditor::Suspend();
								tooltip_pos = io.MousePos;
								ax::NodeEditor::Resume();
							}

							auto linked = false;
							for (auto& l : group->links)
							{
								if (l->to_slot == input)
								{
									linked = true;
									break;
								}
							}
							if (!linked)
							{
								ImGui::PushID(input);
								if (auto bind = group->get_data_bind(input); !bind.empty())
								{
									ImGui::TextUnformatted("=");
									ImGui::SameLine();
									ImGui::SetNextItemWidth(100.f);
									ImGui::InputText("", &bind, ImGuiInputTextFlags_ReadOnly);
								}
								else
								{
									auto changed = manipulate_value(input->type, input->data);
									if (changed)
									{
										input->data_changed_frame = frame;
										group->data_changed_frame = frame;
										blueprint->dirty_frame = frame;
										unsaved = true;
									}
								}
								ImGui::PopID();
							}
						}
					}
					ImGui::EndGroup();
					ImGui::SameLine(0.f, 16.f);
					ImGui::BeginGroup();
					for (auto i = 0; i < n->outputs.size(); i++)
					{
						auto output = n->outputs[i].get();
						if (!output->type || (output->flags & BlueprintSlotFlagHideInUI))
							continue;
						ax::NodeEditor::BeginPin((uint64)output, ax::NodeEditor::PinKind::Output);
						ImGui::Text("%s %s", output->name_hash == "Execute"_h ? "" : output->name.c_str(), graphics::font_icon_str("play"_h).c_str());
						ax::NodeEditor::EndPin();
						if (debugging_instance)
						{
							if (ImGui::IsItemHovered())
							{
								auto& arg = instance_object->outputs[i];
								if (arg.type)
								{
									tooltip = std::format("{} ({})\nObject ID: {}", get_slot_value(arg), ti_str(arg.type), output->object_id);
									ax::NodeEditor::Suspend();
									tooltip_pos = io.MousePos;
									ax::NodeEditor::Resume();
								}
							}
						}
						else
						{
							if (ImGui::IsItemHovered())
							{
								tooltip = std::format("({})\nObject ID: {}", ti_str(output->type), output->object_id);
								ax::NodeEditor::Suspend();
								tooltip_pos = io.MousePos;
								ax::NodeEditor::Resume();
							}
						}
					}
					ImGui::EndGroup();

					if (n->preview_provider)
					{
						BpNodePreview* preview = nullptr;
						if (auto it = previews.find(n.get()); it != previews.end())
							preview = &it->second;
						else
						{
							preview = &previews.emplace(n.get(), BpNodePreview()).first->second;
							preview->model_previewer.init();
						}

						BlueprintNodePreview data;
						n->preview_provider(instance_object->inputs.data(), instance_object->outputs.data(), &data);
						switch (data.type)
						{
						case "image"_h:
						{
							auto image = (graphics::ImagePtr)data.data;
							if (image)
								ImGui::Image(image, vec2(256));
						}
							break;
						case "mesh"_h:
						{
							auto pmesh = (graphics::MeshPtr)data.data;
							if (preview->model_previewer.updated_frame < instance_object->updated_frame)
							{
								auto mesh = preview->model_previewer.model->get_component<cMesh>();
								if (mesh->mesh_res_id != -1)
								{
									app.renderer->release_mesh_res(mesh->mesh_res_id);
									mesh->mesh_res_id = -1;
								}
								mesh->mesh = pmesh;
								mesh->mesh_res_id = app.renderer->get_mesh_res(pmesh);
								mesh->node->mark_transform_dirty();
							}

							preview->model_previewer.update(instance_object->updated_frame);
						}
							break;
						}
					}

					n->position = ax::NodeEditor::GetNodePosition((ax::NodeEditor::NodeId)n.get());

					ax::NodeEditor::EndNode();
					ax::NodeEditor::PopStyleColor();
				}

				for (auto& l : group->links)
					ax::NodeEditor::Link((uint64)l.get(), (uint64)l->from_slot, (uint64)l->to_slot);

				auto mouse_pos = ImGui::GetMousePos();
				static vec2				open_popup_pos;
				static BlueprintSlotPtr	new_node_link_slot = nullptr;

				if (ax::NodeEditor::BeginCreate())
				{
					if (BlueprintSlotPtr from_slot;
						ax::NodeEditor::QueryNewNode((ax::NodeEditor::PinId*)&from_slot))
					{
						if (ax::NodeEditor::AcceptNewItem())
						{
							ax::NodeEditor::Suspend();
							open_popup_pos = mouse_pos;
							new_node_link_slot = from_slot;
							ImGui::OpenPopup("add_node_context_menu");
							ax::NodeEditor::Resume();
						}
					}
					if (BlueprintSlotPtr from_slot, to_slot;
						ax::NodeEditor::QueryNewLink((ax::NodeEditor::PinId*)&from_slot, (ax::NodeEditor::PinId*)&to_slot))
					{
						if (from_slot && to_slot && from_slot != to_slot)
						{
							if (from_slot->flags & BlueprintSlotFlagInput)
								std::swap(from_slot, to_slot);
							if (blueprint_allow_type(to_slot->allowed_types, from_slot->type))
							{
								if (ax::NodeEditor::AcceptNewItem())
								{
									blueprint->add_link(from_slot, to_slot);
									unsaved = true;
								}
							}
						}
					}

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				ax::NodeEditor::EndCreate();
				if (ax::NodeEditor::BeginDelete())
				{
					BlueprintLinkPtr link;
					ax::NodeEditor::NodeId node_id;
					while (ax::NodeEditor::QueryDeletedLink((ax::NodeEditor::LinkId*)&link))
					{
						if (ax::NodeEditor::AcceptDeletedItem())
						{
							blueprint->remove_link(link);
							unsaved = true;
						}
					}
					while (ax::NodeEditor::QueryDeletedNode(&node_id))
					{
						if (ax::NodeEditor::AcceptDeletedItem())
						{
							auto obj = get_object_from_ax_node_id(node_id, instance_group);
							if (obj.type == BlueprintObjectNode)
							{
								if (auto it = previews.find(obj.p.node); it != previews.end())
									previews.erase(it);
								blueprint->remove_node(obj.p.node);
								unsaved = true;
							}
							else if (obj.type == BlueprintObjectBlock)
							{
								blueprint->remove_block(obj.p.block);
								unsaved = true;
							}
						}
					}

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				ax::NodeEditor::EndDelete();

				ax::NodeEditor::NodeId	context_node_id;
				static BlueprintObject	context_object;
				static BlueprintSlotPtr	context_slot = nullptr;
				static BlueprintLinkPtr	context_link = nullptr;

				ax::NodeEditor::Suspend();
				if (ax::NodeEditor::ShowNodeContextMenu(&context_node_id))
				{
					context_object = get_object_from_ax_node_id(context_node_id, instance_group);
					open_popup_pos = mouse_pos;
					ImGui::OpenPopup("node_context_menu");
				}
				else if (ax::NodeEditor::ShowPinContextMenu((ax::NodeEditor::PinId*)&context_slot))
				{
					open_popup_pos = mouse_pos;
					ImGui::OpenPopup("pin_context_menu");
				}
				else if (ax::NodeEditor::ShowLinkContextMenu((ax::NodeEditor::LinkId*)&context_link))
				{
					open_popup_pos = mouse_pos;
					ImGui::OpenPopup("link_context_menu");
				}
				else if (ax::NodeEditor::ShowBackgroundContextMenu())
				{
					open_popup_pos = mouse_pos;
					ImGui::OpenPopup("add_node_context_menu");
				}
				ax::NodeEditor::Resume();

				ax::NodeEditor::Suspend();
				if (ImGui::BeginPopup("node_context_menu"))
				{
					if (ImGui::Selectable("Copy"))
						;
					if (ImGui::Selectable("Duplicate"))
						;
					if (ImGui::Selectable("Delete"))
						;
					if (context_object.type == BlueprintObjectNode)
					{
						if (!blueprint_window.debugger->has_break_node(context_object.p.node))
						{
							if (ImGui::Selectable("Set Break Node"))
								blueprint_window.debugger->add_break_node(context_object.p.node);
						}
						else
						{
							if (ImGui::Selectable("Unset Break Node"))
								blueprint_window.debugger->remove_break_node(context_object.p.node);
						}
					}
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopup("pin_context_menu"))
				{
					if (ImGui::Selectable("Break Links"))
						;
					if (ImGui::Selectable("Reset"))
						;
					if (context_slot->flags & BlueprintSlotFlagInput)
					{
						if (ImGui::BeginMenu("Type"))
						{
							for (auto t : context_slot->allowed_types)
							{
								if (ImGui::Selectable(ti_str(t).c_str()))
									blueprint->set_input_type(context_slot, t);
							}
							ImGui::EndMenu();
						}
						if (auto bind = group->get_data_bind(context_slot); !bind.empty())
						{
							if (ImGui::Selectable("Unbind"))
							{
								for (auto it = group->data_binds.begin(); it != group->data_binds.end(); it++)
								{
									if (it->second == context_slot)
									{
										group->data_binds.erase(it);
										break;
									}
								}
								unsaved = true;
							}
						}
						else
						{
							if (ImGui::Selectable("Bind.."))
							{
								ImGui::OpenInputDialog("Bind", "Bind to an variable from blueprint", [this, group](bool ok, const std::string& str) {
									if (ok)
									{
										auto& bind = group->data_binds.emplace_back();
										bind.first = str;
										bind.second = context_slot;
										unsaved = true;
									}
								});
							}
						}
					}
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopup("link_context_menu"))
				{
					if (ImGui::Selectable("Delete"))
						;
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopup("add_node_context_menu"))
				{
					static auto standard_library = BlueprintNodeLibrary::get(L"standard");
					static auto noise_library = BlueprintNodeLibrary::get(L"graphics::noise");
					static auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
					static auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");
					static auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");
					static auto navigation_library = BlueprintNodeLibrary::get(L"universe::navigation");
					static auto input_library = BlueprintNodeLibrary::get(L"universe::input");
					static auto hud_library = BlueprintNodeLibrary::get(L"universe::HUD");

					static std::string filter = "";
					ImGui::InputText("Filter", &filter);

					auto show_node_template = [&](const std::string& name, const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs, uint& slot_name) 
					{
						if (!filter.empty())
						{
							if (name.find(filter) == std::string::npos)
								return false;
						}
						slot_name = 0;
						if (new_node_link_slot)
						{
							auto& slots = (new_node_link_slot->flags & BlueprintSlotFlagOutput) ? inputs : outputs;
							for (auto& s : slots)
							{
								if (blueprint_allow_type(s.allowed_types, new_node_link_slot->type))
								{
									slot_name = s.name_hash;
									break;
								}
							}
							if (!slot_name)
								return false;
						}
						if (ImGui::Selectable(name.c_str()))
							return true;
					};
					auto show_node_library_templates = [&](BlueprintNodeLibraryPtr library) {
						for (auto& t : library->node_templates)
						{
							uint slot_name = 0;
							if (show_node_template(t.name, t.inputs, t.outputs, slot_name))
							{
								auto n = blueprint->add_node(group, nullptr, t.name, t.display_name, t.inputs, t.outputs,
									t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
								n->position = open_popup_pos;
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

								process_object_moved(n);

								if (new_node_link_slot)
								{
									if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
										blueprint->add_link(new_node_link_slot, n->find_input(slot_name));
									else
										blueprint->add_link(n->find_output(slot_name), new_node_link_slot);
								}

								unsaved = true;
							}
						}
					};
					{
						static BlueprintSlotDesc block_input_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagInput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						static BlueprintSlotDesc block_output_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						uint slot_name = 0;
						if (show_node_template("Block", { block_input_desc }, { block_output_desc }, slot_name))
						{
							auto b = blueprint->add_block(group, nullptr);
							b->position = open_popup_pos;
							b->rect = Rect(vec2(0), vec2(200));
							ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
							ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b, b->rect.size());

							if (new_node_link_slot)
							{
								if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
									blueprint->add_link(new_node_link_slot, b->input.get());
								else
									blueprint->add_link(b->output.get(), new_node_link_slot);
							}

							process_object_moved(b);

							unsaved = true;
						}
					}
					if (ImGui::BeginMenu("Variables"))
					{
						auto show_variables = [&](const std::vector<BlueprintVariable>& variables) {
							for (auto& v : variables)
							{
								uint slot_name = 0;
								if (show_node_template(v.name, {}, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {v.type} } }, slot_name))
								{
									auto n = blueprint->add_variable_node(group, nullptr, v.name_hash);
									n->position = open_popup_pos;
									ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

									if (new_node_link_slot)
										blueprint->add_link(n->outputs.front().get(), new_node_link_slot);

									process_object_moved(n);

									unsaved = true;
								}
								if (show_node_template("Set " + v.name, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {v.type}}}, {}, slot_name))
								{
									auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, true);
									n->position = open_popup_pos;
									ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

									if (new_node_link_slot)
										blueprint->add_link(new_node_link_slot, n->inputs.front().get());

									process_object_moved(n);

									unsaved = true;
								}
							}
						};
						show_variables(blueprint->variables);
						show_variables(group->variables);
						ImGui::EndMenu();
					}
					show_node_library_templates(standard_library);
					show_node_library_templates(noise_library);
					show_node_library_templates(texture_library);
					show_node_library_templates(geometry_library);
					show_node_library_templates(entity_library);
					show_node_library_templates(navigation_library);
					show_node_library_templates(input_library);
					show_node_library_templates(hud_library);

					ImGui::EndPopup();

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				else
					new_node_link_slot = nullptr;

				ax::NodeEditor::Resume();

				ax::NodeEditor::End();
				if (debugging_instance)
					ax::NodeEditor::PopStyleColor();

				if (!tooltip.empty())
				{
					auto mpos = io.MousePos;
					io.MousePos = tooltip_pos;
					ImGui::SetTooltip(tooltip.c_str());
					io.MousePos = mpos;
				}
			}
			ImGui::EndChild();

			ImGui::EndTable();
		}
	}

	ax::NodeEditor::SetCurrentEditor(nullptr);

	ImGui::End();
	if (!opened)
		delete this;
}

BlueprintWindow::BlueprintWindow() :
	Window("Blueprint")
{
	debugger = BlueprintDebugger::create();
	BlueprintDebugger::set_current(debugger);
}

View* BlueprintWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new BlueprintView;
	return nullptr;
}

View* BlueprintWindow::open_view(const std::string& name)
{
	return new BlueprintView(name);
}
