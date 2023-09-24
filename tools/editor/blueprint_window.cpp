#include "blueprint_window.h"
#include "project_window.h"

#include <flame/foundation/sheet.h>
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

bool BlueprintView::is_last_added(uint object_id, bool process_condition)
{
	if (process_condition)
	{
		for (auto it = last_added_objects.begin(); it != last_added_objects.end(); it++)
		{
			if (*it == object_id)
			{
				last_added_objects.erase(it);
				return true;
			}
		}
	}
	return false;
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
		blueprint = Blueprint::get(blueprint_path);
		blueprint_instance = BlueprintInstance::create(blueprint);
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

					auto name = var.name;
					ImGui::SetNextItemWidth(200.f);
					ImGui::InputText("Name", &name);
					if (ImGui::IsItemDeactivatedAfterEdit())
					{
						blueprint->alter_variable(nullptr, var.name_hash, name, var.type);
						unsaved = true;
					}
					ImGui::SetNextItemWidth(200.f);
					if (ImGui::BeginCombo("Type", ti_str(var.type).c_str()))
					{
						if (auto type = show_types_menu(); type)
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
				unsaved = true;
			}
			ImGui::SameLine();

			if (ImGui::Button(graphics::font_icon_str("xmark"_h).c_str()))
			{
				blueprint->remove_group(group);
				group = blueprint->groups.back().get();
				group_name = group->name;
				group_name_hash = group->name_hash;
				unsaved = true;

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
				unsaved = true;

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}

			auto debugging_group = blueprint_window.debugger->debugging &&
				blueprint_window.debugger->debugging->instance->blueprint == blueprint &&
				blueprint_window.debugger->debugging->name == group_name_hash ?
				blueprint_window.debugger->debugging : nullptr;
			auto& instance_group = debugging_group ? *debugging_group : blueprint_instance->groups[group_name_hash];

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
						unsaved = true;
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
							unsaved = true;
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
					{
						blueprint->remove_group_input(group, group->inputs[selected_input].name_hash);
						unsaved = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_input > 0)
					{
						std::swap(group->inputs[selected_input], group->inputs[selected_input - 1]);
						unsaved = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_input != -1 && selected_input < group->inputs.size() - 1)
					{
						std::swap(group->inputs[selected_input], group->inputs[selected_input + 1]);
						unsaved = true;
					}
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
						unsaved = true;
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
							unsaved = true;
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
					{
						blueprint->remove_group_output(group, group->outputs[selected_output].name_hash);
						unsaved = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-up"_h).c_str()))
				{
					if (selected_output > 0)
					{
						std::swap(group->outputs[selected_output], group->outputs[selected_output - 1]);
						unsaved = true;
					}
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::font_icon_str("arrow-down"_h).c_str()))
				{
					if (selected_output != -1 && selected_output < group->outputs.size() - 1)
					{
						std::swap(group->outputs[selected_output], group->outputs[selected_output + 1]);
						unsaved = true;
					}
				}
				ImGui::PopID();

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}
			if (debugging_group)
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
					if (auto o = debugging_group->executing_object(); o && o->original.type == BlueprintObjectNode)
					{
						if (blueprint_window.debugger->has_break_node(o->original.p.node))
						{
							break_node = o->original.p.node;
							blueprint_window.debugger->remove_break_node(break_node);
						}
					}
					debugging_group->instance->step(debugging_group);
					if (break_node)
						blueprint_window.debugger->add_break_node(break_node);
				};

				if (ImGui::Button("Run"))
				{
					if (!debugging_group)
					{
						auto g = blueprint_instance->get_group(group_name_hash);
						blueprint_instance->prepare_executing(g);
						blueprint_instance->run(g);
					}
					else
					{
						step();
						debugging_group->instance->step(debugging_group);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Step"))
				{
					if (!debugging_group)
					{
						auto g = blueprint_instance->get_group(group_name_hash);
						blueprint_instance->prepare_executing(g);
						blueprint_window.debugger->debugging = g;
					}
					else
					{
						step();
						if (!debugging_group->executing_stack.empty())
							blueprint_window.debugger->debugging = debugging_group;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
				{
					if (debugging_group)
					{
						blueprint_window.debugger->debugging = nullptr;
						debugging_group->instance->stop(debugging_group);
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

				if (debugging_group)
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImColor(100, 80, 60, 200));
				ax::NodeEditor::Begin("node_editor");

				auto executing_object = debugging_group ? debugging_group->executing_object() : nullptr;
					
				for (auto& b : group->blocks)
				{
					if (b->depth == 0) // skip root block
						continue;

					ax::NodeEditor::BeginNode((uint64)b.get());
					ImGui::Text("(D%d)", b->depth);

					b->position = ax::NodeEditor::GetNodePosition((ax::NodeEditor::NodeId)b.get());
					auto ax_node = ax_node_editor->GetNodeBuilder().m_CurrentNode;
					{
						auto bounds = ax_node->m_GroupBounds;
						b->rect = Rect(bounds.Min, bounds.Max);

						if (is_last_added(b->object_id, b->rect.a != b->position))
						{
							BlueprintBlockPtr most_depth_block = nullptr;
							uint most_depth = 0;
							for (auto& b2 : group->blocks)
							{
								if (b2->depth == 0) // skip the root block
									continue;
								if (b2.get() != b.get() && b2->rect.contains(b->position))
								{
									if (b2->depth > most_depth)
									{
										most_depth_block = b2.get();
										most_depth = b2->depth;
									}
								}
							}

							if (most_depth_block)
							{
								blueprint->set_block_parent(b.get(), most_depth_block);
								expand_block_sizes();
							}
						}
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
					ImGui::Text("%s (D%d)", display_name.c_str(), n->block->depth + 1);
					ImGui::BeginGroup();
					for (auto i = 0; i < n->inputs.size(); i++)
					{
						auto input = n->inputs[i].get();
						if (input->flags & BlueprintSlotFlagHideInUI)
							continue;
						ax::NodeEditor::BeginPin((uint64)input, ax::NodeEditor::PinKind::Input);
						ImGui::Text("%s %s", graphics::font_icon_str("play"_h).c_str(), input->name_hash == "Execute"_h ? "" : input->name.c_str());
						ax::NodeEditor::EndPin();
						if (debugging_group)
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
								if (auto bind = group->find_data_bind(input); bind)
								{
									ImGui::TextUnformatted("=");
									ImGui::SameLine();
									ImGui::SetNextItemWidth(100.f);
									std::string s = std::format("{}.{}", bind->sheet_name, bind->column_name);
									ImGui::InputText("", &s, ImGuiInputTextFlags_ReadOnly);
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
						if (debugging_group)
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
					auto sz = (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)n.get());
					if (is_last_added(n->object_id, sz.x > 0.f && sz.y > 0.f))
					{
						BlueprintBlockPtr most_depth_block = nullptr;
						uint most_depth = 0;
						for (auto& b2 : group->blocks)
						{
							if (b2->depth == 0) // skip the root block
								continue;
							if (b2->rect.contains(n->position))
							{
								if (b2->depth > most_depth)
								{
									most_depth_block = b2.get();
									most_depth = b2->depth;
								}
							}
						}

						if (most_depth_block)
						{
							blueprint->set_node_block(n.get(), most_depth_block);
							expand_block_sizes();
						}
					}

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

				static std::vector<BlueprintObject> copied_objects;
				auto are_ancestors_in_copied_objects = [](BlueprintObject obj) {
					auto b = obj.get_locate_block();
					while (b)
					{
						for (auto& o : copied_objects)
						{
							if (o.type == BlueprintObjectBlock && o.p.block == b)
								return true;
						}
						b = b->parent;
					}
					return false;
				};

				ax::NodeEditor::Suspend();
				if (ImGui::BeginPopup("node_context_menu"))
				{
					if (ImGui::Selectable("Copy"))
					{
						if (auto n = ax::NodeEditor::GetSelectedObjectCount(); n > 0)
						{
							std::vector<ax::NodeEditor::NodeId> node_ids(n);
							n = ax::NodeEditor::GetSelectedNodes(node_ids.data(), n);
							if (n > 0)
							{
								copied_objects.clear();
								for (auto i = 0; i < n; i++)
								{
									auto obj = get_object_from_ax_node_id(node_ids[i], instance_group);
									if (!are_ancestors_in_copied_objects(obj))
										copied_objects.push_back(obj);
								}
								for (auto& l : group->links)
								{
									// if the link's from_slot is in copied_objects
									if (std::find_if(copied_objects.begin(), copied_objects.end(), [&](const BlueprintObject& obj) {
										return l->from_slot->parent.get_id() == obj.get_id();
									}) != copied_objects.end() || are_ancestors_in_copied_objects(l->from_slot->parent))
									{
										// if the link's to_slot is in copied_objects
										if (std::find_if(copied_objects.begin(), copied_objects.end(), [&](const BlueprintObject& obj) {
											return l->to_slot->parent.get_id() == obj.get_id();
											}) != copied_objects.end() || are_ancestors_in_copied_objects(l->to_slot->parent))
										{
											copied_objects.push_back(l.get());
										}
									}
								}
							}
						}
					}
					if (ImGui::Selectable("Delete"))
					{

					}
					if (ImGui::Selectable("Wrap In Block"))
					{

					}
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
						if (auto bind = group->find_data_bind(context_slot); bind)
						{
							if (ImGui::Selectable("Unbind"))
							{
								for (auto it = group->data_binds.begin(); it != group->data_binds.end(); it++)
								{
									if (it->slot == context_slot)
									{
										it->slot->data_changed_frame = frame;
										group->data_changed_frame = frame;
										blueprint->dirty_frame = frame;
										group->data_binds.erase(it);
										unsaved = true;
										break;
									}
								}
							}
						}
						else
						{
							if (ImGui::Selectable("Bind.."))
							{
								std::vector<std::string> names(app.project_sheets.size());
								for (auto i = 0; i < app.project_sheets.size(); i++)
									names[i] = app.project_sheets[i]->name;
								ImGui::OpenSelectDialog("Bind", "Select a sheet", names, [this, group](int idx) {
									if (idx != -1)
									{
										auto sht = app.project_sheets[idx];
										std::vector<std::string> names(sht->columns.size());
										for (auto i = 0; i < sht->columns.size(); i++)
											names[i] = sht->columns[i].name;
										ImGui::OpenSelectDialog("Bind", "Select a data", names, [this, group, sht](int idx) {
											if (idx != -1)
											{
												auto& bind = group->data_binds.emplace_back();
												bind.sheet_name = sht->name;
												bind.sheet_name_hash = sht->name_hash;
												bind.column_name = sht->columns[idx].name;
												bind.column_name_hash = sht->columns[idx].name_hash;
												bind.slot = context_slot;
												unsaved = true;
											}
										});
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
							if (!SUS::find_case_insensitive(name, filter))
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
								last_added_objects.push_back(n->object_id);
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

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
					if (!copied_objects.empty())
					{
						if (ImGui::MenuItem("Paste"))
						{
							std::map<uint, BlueprintObject> object_map; // the original id to the new object, use for linking
							auto pervious_base_pos = vec2(+10000.f);
							for (auto& obj : copied_objects)
							{
								if (obj.type == BlueprintObjectNode || obj.type == BlueprintObjectBlock)
									pervious_base_pos = min(pervious_base_pos, obj.get_position());
							}
							for (auto& obj : copied_objects)
							{
								auto copy_node = [&](BlueprintNodePtr src_n, BlueprintBlockPtr parent) {
									BlueprintNodePtr n = nullptr;
									if (src_n->name_hash == "Variable"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "get"_h);
									}
									else if (src_n->name_hash == "Set Variable"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "set"_h);
									}
									else if (src_n->name_hash == "Array Size"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "array_size"_h);
									}
									else if (src_n->name_hash == "Array Get Item"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "array_get_item"_h);
									}
									else if (src_n->name_hash == "Array Set Item"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "array_set_item"_h);
									}
									else if (src_n->name_hash == "Array Add Item"_h)
									{
										auto first_input = src_n->inputs.front().get();
										n = blueprint->add_variable_node(group, parent, *(uint*)first_input->data, "array_add_item"_h);
									}
									else
									{
										std::vector<BlueprintSlotDesc> inputs(src_n->inputs.size()), outputs(src_n->outputs.size());
										for (auto i = 0; i < inputs.size(); i++)
										{
											auto& src_s = src_n->inputs[i];
											auto& dst_s = inputs[i];
											dst_s.name = src_s->name;
											dst_s.name_hash = src_s->name_hash;
											dst_s.flags = src_s->flags;
											dst_s.allowed_types = src_s->allowed_types;
										}
										for (auto i = 0; i < outputs.size(); i++)
										{
											auto& src_s = src_n->outputs[i];
											auto& dst_s = outputs[i];
											dst_s.name = src_s->name;
											dst_s.name_hash = src_s->name_hash;
											dst_s.flags = src_s->flags;
											dst_s.allowed_types = src_s->allowed_types;
										}
										n = blueprint->add_node(group, parent, src_n->name, src_n->display_name, inputs, outputs,
											src_n->function, src_n->constructor, src_n->destructor, src_n->input_slot_changed_callback, src_n->preview_provider);
									}
									if (n)
									{
										for (auto i = 0; i < src_n->inputs.size(); i++)
										{
											auto& src_s = src_n->inputs[i];
											auto& dst_s = n->inputs[i];
											if (dst_s->type != src_s->type)
												blueprint->set_input_type(dst_s.get(), src_s->type);
											src_s->type->copy(dst_s->data, src_s->data);
										}

										n->position = open_popup_pos + src_n->position - pervious_base_pos;
										last_added_objects.push_back(n->object_id);
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);
										object_map[src_n->object_id] = n;
									}
									return n;
								};
								if (obj.type == BlueprintObjectNode)
								{
									auto src_n = obj.p.node;
									copy_node(src_n, nullptr);
								}
								else if (obj.type == BlueprintObjectBlock)
								{
									auto src_b = obj.p.block;
									std::function<void(BlueprintBlockPtr, BlueprintBlockPtr)> copy_block;
									copy_block = [&](BlueprintBlockPtr src_b, BlueprintBlockPtr parent) {
										auto b = blueprint->add_block(group, parent);
										for (auto& c : src_b->children)
											copy_block(c, b);
										for (auto& n : src_b->nodes)
											copy_node(n, b);
										b->position = open_popup_pos + src_b->position - pervious_base_pos;
										b->rect = Rect(vec2(0), vec2(0));
										last_added_objects.push_back(b->object_id);
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
										ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b, src_b->rect.size());
										object_map[src_b->object_id] = b;
									};
									copy_block(src_b, nullptr);
								}
							}
							for (auto& obj : copied_objects)
							{
								if (obj.type == BlueprintObjectLink)
								{
									auto src_l = obj.p.link;
									auto& from_object = object_map[src_l->from_slot->parent.get_id()];
									auto& to_object = object_map[src_l->to_slot->parent.get_id()];
									blueprint->add_link(from_object.find_output(src_l->from_slot->name_hash), 
										to_object.find_input(src_l->to_slot->name_hash));
								}

							}

							unsaved = true;
						}
						if (ImGui::MenuItem("Clear Copies"))
							copied_objects.clear();
					}
					{
						static BlueprintSlotDesc block_input_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagInput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						static BlueprintSlotDesc block_output_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						uint slot_name = 0;
						if (show_node_template("Block", { block_input_desc }, { block_output_desc }, slot_name))
						{
							auto b = blueprint->add_block(group, nullptr);
							b->position = open_popup_pos;
							b->rect = Rect(vec2(0), vec2(200));
							last_added_objects.push_back(b->object_id);
							ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
							ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b, b->rect.size());

							if (new_node_link_slot)
							{
								if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
									blueprint->add_link(new_node_link_slot, b->input.get());
								else
									blueprint->add_link(b->output.get(), new_node_link_slot);
							}

							unsaved = true;
						}
					}
					if (ImGui::BeginMenu("Variables"))
					{
						auto show_variables = [&](const std::vector<BlueprintVariable>& variables) {
							for (auto& v : variables)
							{
								uint slot_name = 0;
								if (ImGui::BeginMenu(v.name.c_str()))
								{
									if (show_node_template("Get", {}, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {v.type}} }, slot_name))
									{
										auto n = blueprint->add_variable_node(group, nullptr, v.name_hash);
										n->position = open_popup_pos;
										last_added_objects.push_back(n->object_id);
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

										if (new_node_link_slot)
											blueprint->add_link(n->outputs.front().get(), new_node_link_slot);

										unsaved = true;
									}

									if (is_vector(v.type->tag))
									{
										if (show_node_template("Size", {}, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {TypeInfo::get<uint>()}} }, slot_name))
										{
											auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, "array_size"_h);
											n->position = open_popup_pos;
											last_added_objects.push_back(n->object_id);
											ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

											if (new_node_link_slot)
												blueprint->add_link(n->outputs.front().get(), new_node_link_slot);

											unsaved = true;
										}
										if (show_node_template("Get Item", 
											{ BlueprintSlotDesc{.name = "Index", .name_hash = "Index"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {TypeInfo::get<uint>()}} }, 
											{ BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {v.type->get_wrapped()}} }, slot_name))
										{
											auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, "array_get_item"_h);
											n->position = open_popup_pos;
											last_added_objects.push_back(n->object_id);
											ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

											if (new_node_link_slot)
											{
												if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
													blueprint->add_link(new_node_link_slot, n->inputs[1].get());
												else
													blueprint->add_link(n->outputs[0].get(), new_node_link_slot);
											}

											unsaved = true;
										}
										if (show_node_template("Set Item",
											{ BlueprintSlotDesc{.name = "Index", .name_hash = "Index"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {TypeInfo::get<uint>()}},
											  BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {v.type->get_wrapped()}} },
											{}, slot_name))
										{
											auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, "array_set_item"_h);
											n->position = open_popup_pos;
											last_added_objects.push_back(n->object_id);
											ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

											if (new_node_link_slot)
												blueprint->add_link(new_node_link_slot, n->find_input(slot_name));

											unsaved = true;
										}
										if (show_node_template("Add Item", { BlueprintSlotDesc{.name = "Item", .name_hash = "Item"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {v.type->get_wrapped()}}}, {}, slot_name))
										{
											auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, "array_add_item"_h);
											n->position = open_popup_pos;
											last_added_objects.push_back(n->object_id);
											ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

											if (new_node_link_slot)
												blueprint->add_link(new_node_link_slot, n->inputs[1].get());

											unsaved = true;
										}
									}
									else
									{
										if (!v.name.starts_with("loop_index"))
										{
											if (show_node_template("Set", { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {v.type}} }, {}, slot_name))
											{
												auto n = blueprint->add_variable_node(group, nullptr, v.name_hash, "set"_h);
												n->position = open_popup_pos;
												last_added_objects.push_back(n->object_id);
												ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

												if (new_node_link_slot)
													blueprint->add_link(new_node_link_slot, n->inputs.front().get());

												unsaved = true;
											}
										}
									}

									ImGui::EndMenu();
								}
							}
						};
						show_variables(blueprint->variables);
						show_variables(group->variables);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Call"))
					{
						for (auto& g : blueprint->groups)
						{
							uint slot_name = 0;
							if (g.get() == group || 
								g->name_hash == "start"_h || 
								g->name_hash == "update"_h)
								continue;
							if (show_node_template(g->name, {}, {}, slot_name))
							{
								auto n = blueprint->add_call_node(group, nullptr, g->name_hash);
								n->position = open_popup_pos;
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

								unsaved = true;
							}
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Standard"))
					{
						show_node_library_templates(standard_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Noise"))
					{
						show_node_library_templates(noise_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Texture"))
					{
						show_node_library_templates(texture_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Geometry"))
					{
						show_node_library_templates(geometry_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Entity"))
					{
						show_node_library_templates(entity_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Navigation"))
					{
						show_node_library_templates(navigation_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Input"))
					{
						show_node_library_templates(input_library);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("HUD"))
					{
						show_node_library_templates(hud_library);
						ImGui::EndMenu();
					}

					ImGui::EndPopup();

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				else
					new_node_link_slot = nullptr;

				ax::NodeEditor::Resume();

				ax::NodeEditor::End();
				if (debugging_group)
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
