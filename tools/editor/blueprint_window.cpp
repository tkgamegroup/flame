#include "blueprint_window.h"
#include "project_window.h"

#include <flame/foundation/sheet.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/model.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>

static ImColor color_from_depth(uint depth)
{
	depth -= 1;
	auto shift = depth / 7;
	auto h = (depth % 7) / 7.f * 360.f + shift * 10.f;
	auto color = rgbColor(vec3(h, 0.5f, 1.f));
	return ImColor(color.r, color.g, color.b);
}

void set_offset_recurisely(BlueprintNodePtr n, const vec2& offset) 
{
	n->position += offset;
	ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);
	for (auto c : n->children)
		set_offset_recurisely(c, offset);
};

static std::vector<BlueprintNodePtr> copied_nodes;

static bool if_contains_any_of(BlueprintNodePtr node, const std::vector<BlueprintNodePtr>& list) 
{
	for (auto n : list)
	{
		if (node->contains(n))
			return true;
	}
	return false;
};

static std::vector<BlueprintNodePtr> get_selected_nodes()
{
	std::vector<BlueprintNodePtr> nodes;
	if (auto n = ax::NodeEditor::GetSelectedObjectCount(); n > 0)
	{
		std::vector<ax::NodeEditor::NodeId> node_ids(n);
		n = ax::NodeEditor::GetSelectedNodes(node_ids.data(), n);
		if (n > 0)
		{
			nodes.resize(n);
			for (auto i = 0; i < n; i++)
				nodes[i] = (BlueprintNodePtr)(uint64)node_ids[i];
		}
	}
	return nodes;
}

static BlueprintNodePtr				f9_bound_breakpoint = nullptr;
static BlueprintBreakpointOption	f9_bound_breakpoint_option;

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
	{
		blueprint_path = sp[0];
		if (sp.size() > 2)
		{
			group_name = sp[1];
			group_name_hash = sh(group_name.c_str());
			View::name = std::string(sp[0]) + "##" + std::string(sp[2]);
		}
	}

#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::Config ax_node_editor_config;
	ax_node_editor_config.UserPointer = this;
	ax_node_editor_config.SettingsFile = "";
	ax_node_editor_config.NavigateButtonIndex = 2;
	ax_node_editor_config.SaveNodeSettings = [](ax::NodeEditor::NodeId node_id, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* user_data) {
		auto& view = *(BlueprintView*)user_data;
		if (frames == view.load_frame)
			return true;
		if (blueprint_window.debugger->debugging &&
			blueprint_window.debugger->debugging->instance->blueprint == view.blueprint &&
			blueprint_window.debugger->debugging->name == view.group_name_hash)
			return true;
		auto node = (BlueprintNodePtr)(uint64)node_id;
		if (!view.grapes_mode)
		{
			auto do_expand = false;
			if ((reason & ax::NodeEditor::SaveReasonFlags::AddNode) != ax::NodeEditor::SaveReasonFlags::None)
			{
				auto sz = ax::NodeEditor::GetNodeSize(node_id);
				auto ready = node->is_block ? node->position != node->rect.a : ax::NodeEditor::GetNodeSize(node_id).x != 0.f;
				if (!ready)
				{
					auto ax_node = view.ax_node_editor->FindNode(node_id);
					add_event([&, ax_node]() {
						view.ax_node_editor->MakeDirty(ax::NodeEditor::SaveReasonFlags::AddNode | ax::NodeEditor::SaveReasonFlags::Size, ax_node);
						return false;
					});
					return true;
				}

				auto pos = node->position;
				BlueprintNodePtr most_depth_block = nullptr;
				uint most_depth = 0;
				auto g = node->group;
				for (auto& b : g->nodes)
				{
					if (b->is_block && b->rect.contains(pos))
					{
						if (b->depth > most_depth)
						{
							most_depth_block = b.get();
							most_depth = b->depth;
						}
					}
				}

				auto new_block = most_depth_block ? most_depth_block : g->nodes.front().get();
				if (node->parent != new_block)
					view.blueprint->set_node_parent(node, new_block);

				do_expand = true;
			}
			if (ImGui::IsKeyDown(Keyboard_Alt))
				do_expand = true;
			if (do_expand)
				view.expand_block_sizes();
			view.process_relationships(node);
		}
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
		delete blueprint_instance;
	if (ax_node_editor)
		ax::NodeEditor::DestroyEditor((ax::NodeEditor::EditorContext*)ax_node_editor);
}

void BlueprintView::process_relationships(BlueprintNodePtr n)
{
	if (grapes_mode)
		return;

	auto g = blueprint->find_group(group_name_hash);

	auto try_change_node_block = [&](BlueprintNodePtr node) {
		Rect node_rect;
		node_rect.a = node->position;
		node_rect.b = node_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)node);
		BlueprintNodePtr most_depth_block = nullptr;
		uint most_depth = 0;
		auto g = node->group;
		for (auto& b : g->nodes)
		{
			if (b->is_block && b->rect.contains(node_rect))
			{
				if (b->depth > most_depth)
				{
					most_depth_block = b.get();
					most_depth = b->depth;
				}
			}
		}

		auto new_block = most_depth_block ? most_depth_block : g->nodes.front().get();
		if (node->parent != new_block)
			blueprint->set_node_parent(node, new_block);
	};

	if (!n->is_block)
		try_change_node_block(n);
	else
	{
		for (auto& b : g->nodes)
		{
			if (b == g->nodes.front())
				continue;
			if (!b->is_block)
				continue;

			try_change_node_block(b.get());
		}
		for (auto& n : g->nodes)
		{
			if (!n->is_block)
				try_change_node_block(n.get());
		}
	}
}

void BlueprintView::copy_nodes(BlueprintGroupPtr g)
{
	if (auto selected_nodes = get_selected_nodes(); !selected_nodes.empty())
	{
		copied_nodes.clear();
		for (auto n : selected_nodes)
		{
			if (!if_contains_any_of(n, copied_nodes))
				copied_nodes.push_back(n);
		}
	}
}

void BlueprintView::paste_nodes(BlueprintGroupPtr g, const vec2& pos)
{
	std::map<uint, BlueprintNodePtr> node_map; // the original id to the new node, use for linking
	auto pervious_base_pos = vec2(+10000.f);
	for (auto n : copied_nodes)
		pervious_base_pos = min(pervious_base_pos, n->position);
	for (auto n : copied_nodes)
	{
		std::function<void(BlueprintNodePtr, BlueprintNodePtr)> copy_node;
		copy_node = [&](BlueprintNodePtr src_n, BlueprintNodePtr parent) {

			BlueprintNodePtr n = nullptr;
			if (src_n->name_hash == "Variable"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "get"_h, *(uint*)second_input->data);
			}
			else if (src_n->name_hash == "Set Variable"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "set"_h, *(uint*)second_input->data);
			}
			else if (src_n->name_hash == "Array Size"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "array_size"_h, *(uint*)second_input->data);
			}
			else if (src_n->name_hash == "Array Get Item"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "array_get_item"_h, *(uint*)second_input->data);
			}
			else if (src_n->name_hash == "Array Set Item"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "array_set_item"_h, *(uint*)second_input->data);
			}
			else if (src_n->name_hash == "Array Add Item"_h)
			{
				auto first_input = src_n->inputs[0].get();
				auto second_input = src_n->inputs[1].get();
				n = blueprint->add_variable_node(g, parent, *(uint*)first_input->data, "array_add_item"_h, *(uint*)second_input->data);
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
				n = blueprint->add_node(g, parent, src_n->name, src_n->display_name, inputs, outputs,
					src_n->function, src_n->constructor, src_n->destructor, src_n->input_slot_changed_callback, src_n->preview_provider);
			}
			if (n)
			{
				for (auto& c : src_n->children)
					copy_node(c, n);

				for (auto i = 0; i < src_n->inputs.size(); i++)
				{
					auto& src_s = src_n->inputs[i];
					auto& dst_s = n->inputs[i];
					if (dst_s->type != src_s->type)
						blueprint->set_input_type(dst_s.get(), src_s->type);
					src_s->type->copy(dst_s->data, src_s->data);
				}

				n->position = pos + src_n->position - pervious_base_pos;
				ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);
				if (n->is_block)
				{
					n->rect = Rect(vec2(0), vec2(0));
					ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)n, src_n->rect.size());
				}
				node_map[src_n->object_id] = n;
			}
			return n;
		};

		copy_node(n, nullptr);
	}
	for (auto& l : g->links)
	{
		// if the link's from_slot is in copied_nodes
		if (std::find_if(copied_nodes.begin(), copied_nodes.end(), [&](BlueprintNodePtr n) {
			return l->from_slot->node == n;
			}) != copied_nodes.end() || if_contains_any_of(l->from_slot->node, copied_nodes))
		{
			// if the link's to_slot is in copied_nodes
			if (std::find_if(copied_nodes.begin(), copied_nodes.end(), [&](BlueprintNodePtr n) {
				return l->to_slot->node == n;
				}) != copied_nodes.end() || if_contains_any_of(l->to_slot->node, copied_nodes))
			{
				auto from_node = node_map[l->from_slot->node->object_id];
				auto to_node = node_map[l->to_slot->node->object_id];
				if (from_node && to_node)
					blueprint->add_link(from_node->find_output(l->from_slot->name_hash), to_node->find_input(l->to_slot->name_hash));
			}
		}
	}

	unsaved = true;
}

void BlueprintView::set_parent_to_last_node()
{
	std::vector<BlueprintNodePtr> nodes;
	for (auto n : get_selected_nodes())
	{
		if (!if_contains_any_of(n, nodes))
			nodes.push_back(n);
	}
	if (nodes.size() >= 2)
	{
		if (auto last_node = nodes.back(); last_node->is_block)
		{
			Rect wrap_rect;
			for (auto n : nodes)
			{
				if (n != last_node)
				{
					Rect rect;
					rect.a = n->position;
					rect.b = rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)n);
					rect.expand(10.f);
					wrap_rect.expand(rect);
				}
			}

			if (!grapes_mode)
			{
				if (all(lessThan(last_node->rect.size(), wrap_rect.size())))
				{
					auto expand = wrap_rect.size() - last_node->rect.size();
					last_node->rect.b += expand;
					auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)last_node);
					ax_node->m_GroupBounds.Max += expand;
					ax_node->m_Bounds.Max += expand;
					//ax_node_editor->MakeDirty(ax::NodeEditor::SaveReasonFlags::AddNode, ax_node);
				}
			}

			auto offset = last_node->rect.a - wrap_rect.a;
			for (auto n : nodes)
			{
				if (n != last_node)
				{
					blueprint->set_node_parent(n, last_node);
					set_offset_recurisely(n, offset);
				}
			}
		}
	}
}

static BlueprintInstance::Node* step(BlueprintInstance::Group* debugging_group)
{
	blueprint_window.debugger->debugging = nullptr;

	BlueprintNodePtr breakpoint = nullptr;
	BlueprintBreakpointOption breakpoint_option;
	if (auto n = debugging_group->executing_node(); n)
	{
		if (blueprint_window.debugger->has_break_node(n->original, &breakpoint_option))
		{
			breakpoint = n->original;
			blueprint_window.debugger->remove_break_node(breakpoint);
		}
	}
	auto next_node = debugging_group->instance->step(debugging_group);
	if (breakpoint)
		blueprint_window.debugger->add_break_node(breakpoint, breakpoint_option);
	return next_node;
};

void BlueprintView::run_blueprint(BlueprintInstance::Group* debugging_group)
{
	if (!debugging_group)
	{
		auto g = blueprint_instance->get_group(group_name_hash);
		blueprint_instance->prepare_executing(g);
		blueprint_instance->run(g);
	}
	else
	{
		step(debugging_group);
		debugging_group->instance->run(debugging_group);
	}
}

void BlueprintView::step_blueprint(BlueprintInstance::Group* debugging_group)
{
	if (!debugging_group)
	{
		auto g = blueprint_instance->get_group(group_name_hash);
		blueprint_instance->prepare_executing(g);
		blueprint_window.debugger->debugging = g;
	}
	else
	{
		auto next_node = step(debugging_group);
		if (!debugging_group->executing_stack.empty())
			blueprint_window.debugger->debugging = debugging_group;

		if (next_node)
		{
			auto ax_node = ax_node_editor->GetNode((ax::NodeEditor::NodeId)next_node->original);
			ax_node_editor->NavigateTo(ax_node->GetBounds(), false, 0.f);
		}
	}
}

void BlueprintView::stop_blueprint(BlueprintInstance::Group* debugging_group)
{
	if (debugging_group)
	{
		blueprint_window.debugger->debugging = nullptr;
		debugging_group->instance->stop(debugging_group);
	}
}

void BlueprintView::save_blueprint()
{
	if (unsaved)
	{
		blueprint->save();
		unsaved = false;
	}
}

void BlueprintView::expand_block_sizes()
{
	if (grapes_mode)
		return;
	auto g = blueprint->find_group(group_name_hash);
	std::function<void(BlueprintNodePtr)> fit_block_size;
	fit_block_size = [&](BlueprintNodePtr b) {
		Rect rect = b->rect;

		for (auto& c : b->children)
		{
			if (c->is_block)
				fit_block_size(c);

			Rect block_rect;
			block_rect.a = c->position;
			block_rect.b = block_rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)c);
			block_rect.expand(10.f);
			rect.expand(block_rect);
		}

		if (b != g->nodes.front().get())
		{
			if (any(lessThan(rect.a, b->rect.a)) || any(greaterThan(rect.b, b->rect.b)))
			{
				auto lt_off = b->rect.a - rect.a;
				auto rb_off = rect.b - b->rect.b;
				b->position -= lt_off;
				b->rect = rect;
				ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
				auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)b);
				ax_node->m_GroupBounds.Min = b->rect.a;
				ax_node->m_GroupBounds.Max = b->rect.b;
				ax_node->m_Bounds.Min -= lt_off;
				ax_node->m_Bounds.Max += rb_off;
			}
		}
	};
	fit_block_size(g->nodes.front().get());
}

std::string BlueprintView::get_save_name()
{
	auto sp = SUS::split(name, '#');
	if (sp.size() == 2)
		return std::string(sp[0]) + '#' + group_name + "##" + std::string(sp[1]);
	return name;
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

		ImGui::Checkbox("Grapes Mode", &grapes_mode);
		ImGui::SameLine();
		ImGui::Checkbox("Show Misc", &show_misc);
		ImGui::SameLine();
		if (ImGui::Button("Save"))
			save_blueprint();
		ImGui::SameLine();
		if (ImGui::Button("Zoom To Content"))
			ax::NodeEditor::NavigateToContent(0.f);
		ImGui::SameLine();
		if (ImGui::Button("Zoom To Selection"))
			ax::NodeEditor::NavigateToSelection(true, 0.f);

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
					for (auto& n : group->nodes)
					{
						ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n.get(), n->position);
						if (n->is_block && n != group->nodes.front())
							ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)n.get(), n->rect.size());
					}
					last_group = group;
					load_frame = frame;
				}

				if (ImGui::Button("Run"))
					run_blueprint(debugging_group);
				ImGui::SameLine();
				if (ImGui::Button("Step"))
					step_blueprint(debugging_group);
				ImGui::SameLine();
				if (ImGui::Button("Stop"))
					stop_blueprint(debugging_group);

				auto& io = ImGui::GetIO();
				auto& style = ImGui::GetStyle();
				auto dl = ImGui::GetWindowDrawList();
				std::string tooltip; vec2 tooltip_pos;
				auto get_slot_value = [](const BlueprintAttribute& arg)->std::string {
					if (arg.type->tag == TagD || is_pointer(arg.type->tag))
						return std::format("Value: {}", arg.type->serialize(arg.data));
					return "";
				};

				if (debugging_group)
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImColor(100, 80, 60, 200));
				ax::NodeEditor::Begin("node_editor");

				auto executing_node = debugging_group ? debugging_group->executing_node() : nullptr;

				for (auto& n : group->nodes)
				{
					if (n == group->nodes.front()) // skip root block
						continue;

					auto instance_node = instance_group.node_map[n->object_id];

					auto display_name = n->display_name.empty() ? n->name : n->display_name;
					auto bg_color = ax::NodeEditor::GetStyle().Colors[ax::NodeEditor::StyleColor_NodeBg];
					auto border_color = color_from_depth(n->depth);
					if (blueprint_window.debugger->has_break_node(n.get()))
					{
						if (executing_node && executing_node->original == n.get())
						{
							display_name = "=> " + display_name;
							bg_color = ImColor(204, 116, 45, 200);
						}
						else
							bg_color = ImColor(197, 81, 89, 200);
					}
					else if (executing_node && executing_node->original == n.get())
					{
						display_name = "=> " + display_name;
						bg_color = ImColor(211, 151, 0, 200);
					}
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, bg_color);
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_NodeBorder, border_color);

					ax::NodeEditor::BeginNode((uint64)n.get());
					if (show_misc)
					{
						vec2 pos = ImGui::GetCursorPos();
						pos.y -= ImGui::GetTextLineHeight();
						pos.y -= style.FramePadding.y * 2;
						auto text = std::format("O{}D{}", instance_node->order, n->depth);
						dl->AddText(pos, ImColor(1.f, 1.f, 1.f), text.c_str());
					}
					ImGui::TextUnformatted(display_name.c_str());
					ImGui::BeginGroup();
					for (auto i = 0; i < n->inputs.size(); i++)
					{
						auto input = n->inputs[i].get();
						if (input->flags & BlueprintSlotFlagHideInUI)
							continue;
						ax::NodeEditor::BeginPin((uint64)input, ax::NodeEditor::PinKind::Input);
						ImGui::Text("%s %s", graphics::font_icon_str("play"_h).c_str(), input->name_hash == "Execute"_h ? "" : input->name.c_str());
						ax::NodeEditor::EndPin();
						if (ImGui::IsItemHovered())
						{
							if (debugging_group)
							{
								if (i < instance_node->inputs.size())
								{
									auto& arg = instance_node->inputs[i];
									if (arg.type == input->type)
										tooltip = std::format("{} ({})\nObject ID: {}", get_slot_value(arg), ti_str(arg.type), input->object_id);
								}
							}
							else
								tooltip = std::format("({})\nObject ID: {}", ti_str(input->type), input->object_id);
							ax::NodeEditor::Suspend();
							tooltip_pos = io.MousePos;
							ax::NodeEditor::Resume();
						}
						if (!input->is_linked())
						{
							if (debugging_group)
								ImGui::BeginDisabled();
							ImGui::PushID(input);
							if (manipulate_value(input->type, input->data))
							{
								input->data_changed_frame = frame;
								group->data_changed_frame = frame;
								blueprint->dirty_frame = frame;
								unsaved = true;
							}
							ImGui::PopID();
							if (debugging_group)
								ImGui::EndDisabled();
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
						if (ImGui::IsItemHovered())
						{
							if (debugging_group)
							{
								if (i < instance_node->outputs.size())
								{
									auto& arg = instance_node->outputs[i];
									if (arg.type == output->type)
										tooltip = std::format("{} ({})\nObject ID: {}", get_slot_value(arg), ti_str(arg.type), output->object_id);
								}
							}
							else
								tooltip = std::format("({})\nObject ID: {}", ti_str(output->type), output->object_id);
							ax::NodeEditor::Suspend();
							tooltip_pos = io.MousePos;
							ax::NodeEditor::Resume();
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
						n->preview_provider(instance_node->inputs.data(), instance_node->outputs.data(), &data);
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
							if (preview->model_previewer.updated_frame < instance_node->updated_frame)
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

							preview->model_previewer.update(instance_node->updated_frame);
						}
							break;
						}
					}

					auto ax_node = ax_node_editor->GetNodeBuilder().m_CurrentNode;
					vec2 new_pos = ax_node->m_Bounds.Min;
					if (!grapes_mode)
						n->position = new_pos;
					else if (n->position != new_pos)
					{
						if (!ImGui::IsKeyDown(Keyboard_Alt))
						{
							if (n->is_block)
							{
								auto offset = new_pos - n->position;
								set_offset_recurisely(n.get(), offset);
							}
						}
						n->position = new_pos;
					}

					if (!grapes_mode)
					{
						if (n->is_block)
						{
							auto bounds = ax_node->m_GroupBounds;
							n->rect = Rect(bounds.Min, bounds.Max); // get rect from last frame

							ax::NodeEditor::Group(n->rect.size());
						}

						if (n->is_block)
						{
							// restore last pin
							auto last_pin = ax_node->m_LastPin;
							ax::NodeEditor::EndNode();
							// recover last pin and re-active all pins, since our groups(blocks) can have pins
							ax_node->m_LastPin = last_pin;
							for (auto pin = ax_node->m_LastPin; pin; pin = pin->m_PreviousPin)
								pin->m_IsLive = true;
						}
						else
							ax::NodeEditor::EndNode();
					}
					else
					{
						if (n->is_block)
						{
							auto col = color_from_depth(n->depth + 1);
							ImGui::InvisibleButton("block", ImVec2(80, 4));
							auto p0 = ImGui::GetItemRectMin();
							auto p1 = ImGui::GetItemRectMax();
							dl->AddRectFilled(p0, p1, col);
							col.Value.w = 0.3f;

							for (auto c : n->children)
								dl->AddLine(c->position, vec2((p0 + p1) * 0.5f), col);
						}
						ax::NodeEditor::EndNode();
					}

					ax::NodeEditor::PopStyleColor(2);
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
							open_popup_pos = floor((vec2)mouse_pos);
							new_node_link_slot = from_slot;
							ImGui::OpenPopup("add_node_context_menu");
						}
					}
					if (BlueprintSlotPtr from_slot, to_slot;
						ax::NodeEditor::QueryNewLink((ax::NodeEditor::PinId*)&from_slot, (ax::NodeEditor::PinId*)&to_slot))
					{
						if (from_slot && to_slot && from_slot != to_slot)
						{
							if (from_slot->flags & BlueprintSlotFlagInput)
								std::swap(from_slot, to_slot);
							if (!(to_slot->flags & BlueprintSlotFlagOutput))
							{
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
							auto node = (BlueprintNodePtr)(uint64)node_id;

							if (auto it = previews.find(node); it != previews.end())
								previews.erase(it);
							blueprint->remove_node(node);
							unsaved = true;
						}
					}

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				ax::NodeEditor::EndDelete();

				ax::NodeEditor::NodeId	context_node_id;
				static BlueprintNodePtr	context_node;
				static BlueprintSlotPtr	context_slot = nullptr;
				static BlueprintLinkPtr	context_link = nullptr;

				ax::NodeEditor::Suspend();
				if (ax::NodeEditor::ShowNodeContextMenu(&context_node_id))
				{
					context_node = (BlueprintNodePtr)(uint64)context_node_id;
					open_popup_pos = floor((vec2)mouse_pos);
					ImGui::OpenPopup("node_context_menu");
				}
				else if (ax::NodeEditor::ShowPinContextMenu((ax::NodeEditor::PinId*)&context_slot))
				{
					open_popup_pos = floor((vec2)mouse_pos);
					ImGui::OpenPopup("pin_context_menu");
				}
				else if (ax::NodeEditor::ShowLinkContextMenu((ax::NodeEditor::LinkId*)&context_link))
				{
					open_popup_pos = floor((vec2)mouse_pos);
					ImGui::OpenPopup("link_context_menu");
				}
				else if (ax::NodeEditor::ShowBackgroundContextMenu())
				{
					open_popup_pos = floor((vec2)mouse_pos);
					ImGui::OpenPopup("add_node_context_menu");
				}
				ax::NodeEditor::Resume();

				ax::NodeEditor::Suspend();
				if (ImGui::BeginPopup("node_context_menu"))
				{
					if (ImGui::Selectable("Copy"))
						copy_nodes(group);
					if (ImGui::Selectable("Cut"))
					{

					}
					if (ImGui::Selectable("Delete"))
					{

					}
					if (ImGui::Selectable("Wrap In Block"))
					{
						if (auto selected_nodes = get_selected_nodes(); !selected_nodes.empty())
						{
							std::vector<BlueprintNodePtr> nodes;
							for (auto n : selected_nodes)
							{
								if (!if_contains_any_of(n, nodes))
									nodes.push_back(n);
							}

							Rect wrap_rect;
							for (auto n : nodes)
							{
								Rect rect;
								rect.a = n->position;
								rect.b = rect.a + (vec2)ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)n);
								rect.expand(10.f);
								wrap_rect.expand(rect);
							}

							auto b = blueprint->add_block(group, nullptr);
							b->position = wrap_rect.a - vec2(10.f, 45.f);
							wrap_rect.b += vec2(5.f, 10.f);
							b->rect = wrap_rect;
							ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
							ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b, b->rect.size());

							for (auto n : nodes)
								blueprint->set_node_parent(n, b);

							unsaved = true;
						}
					}
					if (context_node->is_block)
					{
						if (ImGui::Selectable("Unwrap Block"))
							blueprint->remove_node(context_node, false);
					}
					if (auto n = ax::NodeEditor::GetSelectedObjectCount(); n >= 2)
					{
						ax::NodeEditor::NodeId node_ids[2];
						if (ax::NodeEditor::GetSelectedNodes(node_ids, 2) == 2)
						{
							if (ImGui::Selectable("Set Parent To Last Node"))
								set_parent_to_last_node();
						}
					}
					BlueprintBreakpointOption breakpoint_option;
					if (!blueprint_window.debugger->has_break_node(context_node, &breakpoint_option))
					{
						if (ImGui::BeginMenu("Breakpoint"))
						{
							if (ImGui::Selectable("Set Normal"))
								blueprint_window.debugger->add_break_node(context_node);
							if (ImGui::Selectable("Set Once"))
								blueprint_window.debugger->add_break_node(context_node, BlueprintBreakpointTriggerOnce);
							if (ImGui::Selectable("Set Break In Code"))
								blueprint_window.debugger->add_break_node(context_node, BlueprintBreakpointBreakInCode);
							if (ImGui::BeginMenu("Bind To F9"))
							{
								if (ImGui::Selectable("Set Normal"))
								{
									f9_bound_breakpoint = context_node;
									f9_bound_breakpoint_option = BlueprintBreakpointNormal;
								}
								if (ImGui::Selectable("Set Once"))
								{
									f9_bound_breakpoint = context_node;
									f9_bound_breakpoint_option = BlueprintBreakpointTriggerOnce;
								}
								if (ImGui::Selectable("Set Break In Code"))
								{
									f9_bound_breakpoint = context_node;
									f9_bound_breakpoint_option = BlueprintBreakpointBreakInCode;
								}
								ImGui::EndMenu();
							}
							ImGui::EndMenu();
						}
					}
					else
					{
						if (ImGui::Selectable("Unset"))
							blueprint_window.debugger->remove_break_node(context_node);
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

					auto show_node_template = [&](const std::string& name, const std::vector<BlueprintSlotDesc>& inputs, const std::vector<BlueprintSlotDesc>& outputs, uint& slot_name) {
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
						return true;
					};
					auto show_node_library_template = [&](BlueprintNodeLibrary::NodeTemplate& t) {
						uint slot_name = 0;
						if (show_node_template(t.name, t.inputs, t.outputs, slot_name))
						{
							if (ImGui::Selectable(t.name.c_str()))
							{
								auto n = blueprint->add_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, t.name, t.display_name, t.inputs, t.outputs,
									t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider,
									t.is_block, t.begin_block_function, t.end_block_function);
								n->position = open_popup_pos;
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);
								if (n->is_block)
									ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)n, n->rect.size());

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
					if (!copied_nodes.empty())
					{
						if (ImGui::MenuItem("Paste"))
							paste_nodes(group, open_popup_pos);
						if (ImGui::MenuItem("Clear Copies"))
							copied_nodes.clear();
					}
					{
						static BlueprintSlotDesc block_input_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagInput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						static BlueprintSlotDesc block_output_desc{ .name = "Execute", .name_hash = "Execute"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = { TypeInfo::get<BlueprintSignal>() } };
						uint slot_name = 0;
						if (show_node_template("Block", { block_input_desc }, { block_output_desc }, slot_name))
						{
							if (ImGui::Selectable("Block"))
							{
								auto b = blueprint->add_block(group, nullptr);
								b->position = open_popup_pos;
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
								ax::NodeEditor::SetGroupSize((ax::NodeEditor::NodeId)b, b->rect.size());
								auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)b);
								{
									auto bounds = ax_node->m_GroupBounds;
									b->rect = Rect(bounds.Min, bounds.Max);
								}

								if (new_node_link_slot)
								{
									if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
										blueprint->add_link(new_node_link_slot, b->inputs.front().get());
									else
										blueprint->add_link(b->outputs.front().get(), new_node_link_slot);
								}

								unsaved = true;
							}
						}
					}
					if (ImGui::BeginMenu("Variables"))
					{
						auto show_variable = [&](const std::string& name, uint name_hash, TypeInfo* type, uint location_name) {
							uint slot_name = 0;
							std::vector<std::pair<std::string, std::function<void()>>> actions;
							if (show_node_template(name, {}, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {type}} }, slot_name))
							{
								actions.emplace_back("Get", [&]() {
									auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "get"_h, location_name);
									n->position = open_popup_pos;
									ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

									if (new_node_link_slot)
										blueprint->add_link(n->outputs.front().get(), new_node_link_slot);

									unsaved = true;
								});
							}

							if (is_vector(type->tag))
							{
								if (show_node_template(name, {}, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {TypeInfo::get<uint>()}} }, slot_name))
								{
									actions.emplace_back("Size", [&]() {
										auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "array_size"_h, location_name);
										n->position = open_popup_pos;
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

										if (new_node_link_slot)
											blueprint->add_link(n->outputs.front().get(), new_node_link_slot);

										unsaved = true;
									});
								}
								if (show_node_template(name,
									{ BlueprintSlotDesc{.name = "Index", .name_hash = "Index"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {TypeInfo::get<uint>()}} },
									{ BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagOutput, .allowed_types = {type->get_wrapped()}} }, slot_name))
								{
									actions.emplace_back("Get Item", [&]() {
										auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "array_get_item"_h, location_name);
										n->position = open_popup_pos;
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

										if (new_node_link_slot)
										{
											if (new_node_link_slot->flags & BlueprintSlotFlagOutput)
												blueprint->add_link(new_node_link_slot, n->inputs[1].get());
											else
												blueprint->add_link(n->outputs[0].get(), new_node_link_slot);
										}

										unsaved = true;
									});
								}
								if (show_node_template(name,
									{ BlueprintSlotDesc{.name = "Index", .name_hash = "Index"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {TypeInfo::get<uint>()}},
									  BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {type->get_wrapped()}} },
									{}, slot_name))
								{
									actions.emplace_back("Set Item", [&]() {
										auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "array_set_item"_h, location_name);
										n->position = open_popup_pos;
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

										if (new_node_link_slot)
											blueprint->add_link(new_node_link_slot, n->find_input(slot_name));

										unsaved = true;
									});
								}
								if (show_node_template(name, { BlueprintSlotDesc{.name = "Item", .name_hash = "Item"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {type->get_wrapped()}} }, {}, slot_name))
								{
									actions.emplace_back("Add Item", [&]() {
										auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "array_add_item"_h, location_name);
										n->position = open_popup_pos;
										ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

										if (new_node_link_slot)
											blueprint->add_link(new_node_link_slot, n->inputs[1].get());

										unsaved = true;
									});
								}
							}
							else
							{
								if (!name.starts_with("loop_index"))
								{
									if (show_node_template(name, { BlueprintSlotDesc{.name = "V", .name_hash = "V"_h, .flags = BlueprintSlotFlagInput, .allowed_types = {type}} }, {}, slot_name))
									{
										actions.emplace_back("Set", [&]() {
											auto n = blueprint->add_variable_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, name_hash, "set"_h, location_name);
											n->position = open_popup_pos;
											ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

											if (new_node_link_slot)
												blueprint->add_link(new_node_link_slot, n->inputs.front().get());

											unsaved = true;
										});
									}
								}
							}
							if (!actions.empty())
							if (ImGui::BeginMenu(name.c_str()))
							{
								for (auto& item : actions)
								{
									if (ImGui::Selectable(item.first.c_str()))
										item.second();
								}
								ImGui::EndMenu();
							}
						};
						for (auto& v : blueprint->variables)
							show_variable(v.name, v.name_hash, v.type, 0);
						for (auto& v : group->variables)
							show_variable(v.name, v.name_hash, v.type, 0);
						if (ImGui::BeginMenu("Sheets"))
						{
							for (auto sht : app.project_static_sheets)
							{
								if (ImGui::BeginMenu(sht->name.c_str()))
								{
									for (auto& col : sht->columns)
										show_variable(col.name, col.name_hash, col.type, sht->name_hash);
									ImGui::EndMenu();
								}
							}
							ImGui::EndMenu();
						}
						if (ImGui::BeginMenu("Other Blueprints"))
						{
							for (auto bp : app.project_static_blueprints)
							{
								if (bp == blueprint)
									continue;
								if (ImGui::BeginMenu(bp->name.c_str()))
								{
									for (auto& v : bp->variables)
										show_variable(v.name, v.name_hash, v.type, bp->name_hash);
									ImGui::EndMenu();
								}
							}
							ImGui::EndMenu();
						}
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
								if (ImGui::Selectable(g->name.c_str()))
								{
									auto n = blueprint->add_call_node(group, new_node_link_slot ? new_node_link_slot->node->parent : nullptr, g->name_hash);
									n->position = open_popup_pos;
									ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

									unsaved = true;
								}
							}
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Standard"))
					{
						for (auto i = 0; i < standard_library->node_templates.size(); i++)
						{
							auto& t = standard_library->node_templates[i];
							if (t.name == "Get BP bool")
							{
								if (ImGui::BeginMenu("Get BP"))
								{
									for (auto j = i; ; j++)
									{
										auto& t = standard_library->node_templates[j];
										if (!t.name.starts_with("Get BP "))
											break;
										show_node_library_template(t);
									}
									ImGui::EndMenu();
								}
								continue;
							}
							if (t.name == "Set BP bool")
							{
								if (ImGui::BeginMenu("Set BP"))
								{
									for (auto j = i; ; j++)
									{
										auto& t = standard_library->node_templates[j];
										if (!t.name.starts_with("Set BP "))
											break;
										show_node_library_template(t);
									}
									ImGui::EndMenu();
								}
								continue;
							}
							if (t.name == "Branch 2")
							{
								if (ImGui::BeginMenu("Branch"))
								{
									for (auto j = i; ; j++)
									{
										auto& t = standard_library->node_templates[j];
										if (!t.name.starts_with("Branch "))
											break;
										show_node_library_template(t);
									}
									ImGui::EndMenu();
								}
								continue;
							}
							if (t.name == "Select Branch 2")
							{
								if (ImGui::BeginMenu("Select Branch"))
								{
									for (auto j = i; ; j++)
									{
										auto& t = standard_library->node_templates[j];
										if (!t.name.starts_with("Select Branch "))
											break;
										show_node_library_template(t);
									}
									ImGui::EndMenu();
								}
								continue;
							}
							if (t.name == "Ramp Branch 2")
							{
								if (ImGui::BeginMenu("Ramp Branch"))
								{
									for (auto j = i; ; j++)
									{
										auto& t = standard_library->node_templates[j];
										if (!t.name.starts_with("Ramp Branch "))
											break;
										show_node_library_template(t);
									}
									ImGui::EndMenu();
								}
								continue;
							}
							if (t.name.starts_with("Get BP "))
								continue;
							if (t.name.starts_with("Set BP "))
								continue;
							if (t.name.starts_with("Branch "))
								continue;
							if (t.name.starts_with("Select Branch "))
								continue;
							if (t.name.starts_with("Ramp Branch "))
								continue;
							show_node_library_template(t);
						}
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Noise"))
					{
						for (auto& t : noise_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Texture"))
					{
						for (auto& t : texture_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Geometry"))
					{
						for (auto& t : geometry_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Entity"))
					{
						for (auto& t : entity_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Navigation"))
					{
						for (auto& t : navigation_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("Input"))
					{
						for (auto& t : input_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}
					if (ImGui::BeginMenu("HUD"))
					{
						for (auto& t : hud_library->node_templates)
							show_node_library_template(t);
						ImGui::EndMenu();
					}

					ImGui::EndPopup();
				}
				else
					new_node_link_slot = nullptr;

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();

				ax::NodeEditor::Resume();

				if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows))
				{
					if (!io.WantCaptureKeyboard)
					{
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_C))
							copy_nodes(group);
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_V))
							paste_nodes(group, mouse_pos);
						if (ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_Left))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
							{
								if (n->is_block)
								{
									auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)n);
									ax_node->m_GroupBounds.Max.x -= 10.f;
									ax_node->m_Bounds.Max.x -= 10.f;
								}
							}
						}
						if (ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_Right))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
							{
								if (n->is_block)
								{
									auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)n);
									ax_node->m_GroupBounds.Max.x += 10.f;
									ax_node->m_Bounds.Max.x += 10.f;
								}
							}
						}
						if (ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_Up))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
							{
								if (n->is_block)
								{
									auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)n);
									ax_node->m_GroupBounds.Max.y -= 10.f;
									ax_node->m_Bounds.Max.y -= 10.f;
								}
							}
						}
						if (ImGui::IsKeyDown(Keyboard_Shift) && ImGui::IsKeyPressed(Keyboard_Down))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
							{
								if (n->is_block)
								{
									auto ax_node = ax_node_editor->FindNode((ax::NodeEditor::NodeId)n);
									ax_node->m_GroupBounds.Max.y += 10.f;
									ax_node->m_Bounds.Max.y += 10.f;
								}
							}
						}
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Left))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position + vec2(-10.f, 0.f));
						}
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Right))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position + vec2(+10.f, 0.f));
						}
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Up))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position + vec2(0.f, -10.f));
						}
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_Down))
						{
							auto nodes = get_selected_nodes();
							for (auto n : nodes)
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position + vec2(0.f, +10.f));
						}
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_P))
							set_parent_to_last_node();
						if (ImGui::IsKeyPressed(Keyboard_F10))
							step_blueprint(debugging_group);
						if (ImGui::IsKeyDown(Keyboard_Ctrl) && ImGui::IsKeyPressed(Keyboard_S))
							save_blueprint();
					}
				}

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

void BlueprintView::on_global_shortcuts()
{
	if (ImGui::IsKeyPressed(Keyboard_F9))
	{
		if (f9_bound_breakpoint)
		{
			blueprint_window.debugger->add_break_node(f9_bound_breakpoint, f9_bound_breakpoint_option);
			f9_bound_breakpoint = nullptr;
		}
	}
}

BlueprintWindow::BlueprintWindow() :
	Window("Blueprint")
{
	debugger = BlueprintDebugger::create();
	debugger->callbacks.add([this](uint msg, void* parm1, void* parm2) {
		if (msg == "breakpoint_triggered"_h)
		{
			auto node = (BlueprintNodePtr)parm1;
			auto blueprint = node->group->blueprint;
			for (auto& v : views)
			{
				auto view = (BlueprintView*)v.get();
				if (blueprint == view->blueprint)
				{
					auto ax_node = view->ax_node_editor->GetNode((ax::NodeEditor::NodeId)node);
					view->ax_node_editor->NavigateTo(ax_node->GetBounds(), false, 0.f);
					break;
				}
			}
		}
	});
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
