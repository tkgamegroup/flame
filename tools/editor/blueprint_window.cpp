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
	ax::NodeEditor::Config im_editor_config;
	im_editor_config.UserPointer = this;
	im_editor_config.SettingsFile = "";
	im_editor_config.NavigateButtonIndex = 2;
	im_editor_config.SaveNodeSettings = [](ax::NodeEditor::NodeId node_id, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* user_data) {
		auto& view = *(BlueprintView*)user_data;
		if (frames == view.load_frame)
			return true;
		auto& g = view.blueprint_instance->groups[view.group_name_hash];
		auto obj = get_object_from_ax_node_id(node_id, g);
		view.process_object_moved(obj);
		return true;
	};
	ax_node_editor = (ax::NodeEditor::Detail::EditorContext*)ax::NodeEditor::CreateEditor(&im_editor_config);
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
	auto try_change_node_block = [this](BlueprintNodePtr node) {
		vec2 node_size = ax::NodeEditor::GetNodeSize((ax::NodeEditor::NodeId)node);
		Rect node_rect;
		node_rect.a = node->position;
		node_rect.b = node_rect.a + node_size;
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
		blueprint->set_node_block(node, most_depth_block ? most_depth_block : g->blocks.front().get());
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

			BlueprintBlockPtr most_depth_parent = nullptr;
			uint most_depth = 0;
			for (auto& b2 : g->blocks)
			{
				if (b2->depth == 0) // skip the root block
					continue;

				if (b2->rect.contains(b->rect))
				{
					if (b2->depth > most_depth)
					{
						most_depth_parent = b2.get();
						most_depth = b2->depth;
					}
				}
			}
			blueprint->set_block_parent(b.get(), most_depth_parent ? most_depth_parent : g->blocks.front().get());
		}
		for (auto& n : g->nodes)
			try_change_node_block(n.get());
	}
		break;
	}
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
	ImGui::Begin(name.c_str(), &opened);

	ax::NodeEditor::SetCurrentEditor((ax::NodeEditor::EditorContext*)ax_node_editor);

	if (!blueprint)
	{
		load_blueprint(blueprint_path);
		load_frame = frames;
	}
	if (blueprint)
	{
		if (blueprint_instance->built_frame < blueprint->dirty_frame)
			blueprint_instance->build();

		auto group = blueprint->find_group(group_name_hash);
		if (!group)
		{
			group = blueprint->groups[0].get();
			group_name = group->name;
			group_name_hash = group->name_hash;
		}

		if (ImGui::Button("Save"))
		{
			blueprint->save();
		}

		ImGui::SetNextItemWidth(100.f);
		ImGui::InputText("##group", &group_name);
		if (ImGui::IsItemDeactivatedAfterEdit())
		{
			group_name_hash = sh(group_name.c_str());
			group->name = group_name;
			group->name_hash = group_name_hash;
			auto frame = frames;
			group->structure_changed_frame = frame;
			blueprint->dirty_frame = frame;
		}
		ImGui::SameLine();
		if (ImGui::BeginCombo("##group_dropdown", "", ImGuiComboFlags_NoPreview | ImGuiComboFlags_PopupAlignLeft))
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

		if (ImGui::Button(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
		{
			blueprint->remove_group(group);
			group = nullptr;

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

		if (ImGui::BeginTable("bp_editor", 2, ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::BeginChild("side_panel", ImVec2(0, -2));
			if (ImGui::CollapsingHeader("Inputs:"))
			{
				static int selected_input = -1;
				if (ImGui::BeginListBox("##inputs"))
				{
					if (group)
					{
						for (auto i = 0; i < group->inputs.size(); i++)
						{
							if (ImGui::Selectable(group->inputs[i]->name.c_str(), selected_input == i))
								selected_input = i;
						}
					}
					ImGui::EndListBox();
				}
				selected_input = min(selected_input, (int)group->inputs.size() - 1);

				if (ImGui::SmallButton("+"))
				{
					auto name = get_unique_name("new_input", [&](const std::string& name) {
						for (auto& i : group->inputs)
						{
							if (i->name == name)
								return true;
						}
						return false;
					});
					blueprint->add_group_input(group, name, TypeInfo::get<float>());
					selected_input = group->inputs.size() - 1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("-"))
				{
					if (selected_input != -1)
						blueprint->remove_group_input(group, group->inputs[selected_input].get());
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("arrow-up"_h).c_str()))
				{
					if (selected_input > 0)
						std::swap(group->inputs[selected_input], group->inputs[selected_input - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("arrow-down"_h).c_str()))
				{
					if (selected_input != -1 && selected_input < group->inputs.size() - 1)
						std::swap(group->inputs[selected_input], group->inputs[selected_input + 1]);
				}

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}
			if (ImGui::CollapsingHeader("Outputs:"))
			{
				static int selected_output = -1;
				if (ImGui::BeginListBox("##outputs"))
				{
					for (auto i = 0; i < group->outputs.size(); i++)
					{
						if (ImGui::Selectable(group->outputs[i]->name.c_str(), selected_output == i))
							selected_output = i;
					}
					ImGui::EndListBox();
				}
				selected_output = min(selected_output, (int)group->outputs.size() - 1);

				if (ImGui::SmallButton("+"))
				{
					auto name = get_unique_name("new_output", [&](const std::string& name) {
						for (auto& o : group->outputs)
						{
							if (o->name == name)
								return true;
						}
						return false;
					});
					blueprint->add_group_output(group, name, TypeInfo::get<float>());
					selected_output = group->outputs.size() - 1;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("-"))
				{
					if (selected_output != -1)
						blueprint->remove_group_output(group, group->inputs[selected_output].get());
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("arrow-up"_h).c_str()))
				{
					if (selected_output > 0)
						std::swap(group->outputs[selected_output], group->outputs[selected_output - 1]);
				}
				ImGui::SameLine();
				if (ImGui::SmallButton(graphics::FontAtlas::icon_s("arrow-down"_h).c_str()))
				{
					if (selected_output != -1 && selected_output < group->outputs.size() - 1)
						std::swap(group->outputs[selected_output], group->outputs[selected_output + 1]);
				}

				if (blueprint_instance->built_frame < blueprint->dirty_frame)
					blueprint_instance->build();
			}
			ImGui::EndChild();

			ImGui::TableSetColumnIndex(1);
			ImGui::BeginChild("main_area", ImVec2(0, -2));
			{
				static BlueprintGroupPtr last_group = nullptr;
				if (group != last_group)
				{
					for (auto& n : group->nodes)
						ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n.get(), n->position);
					last_group = group;
				}

				if (ImGui::Button("Run"))
				{
					if (blueprint_instance->executing_stack.empty())
						blueprint_instance->prepare_executing(blueprint_instance->get_group(group_name_hash));
					blueprint_instance->run();
				}
				ImGui::SameLine();
				if (ImGui::Button("Step"))
				{
					if (blueprint_instance->executing_stack.empty())
					{
						blueprint_instance->prepare_executing(blueprint_instance->get_group(group_name_hash));
						blueprint_window.debugger->debugging = blueprint_instance;
					}
					else
					{
						BlueprintNodePtr break_node = nullptr;
						if (auto o = blueprint_instance->executing_object(); o && o->original.type == BlueprintObjectNode)
						{
							if (blueprint_window.debugger->has_break_node(o->original.p.node))
							{
								break_node = o->original.p.node;
								blueprint_window.debugger->remove_break_node(break_node);
							}
						}
						blueprint_window.debugger->debugging = nullptr;
						blueprint_instance->step();
						if (break_node)
							blueprint_window.debugger->add_break_node(break_node);
						if (!blueprint_instance->executing_stack.empty())
							blueprint_window.debugger->debugging = blueprint_instance;
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
				auto get_slot_value = [](const BlueprintArgument& arg)->std::string {
					if (arg.type->tag != TagD)
						return "";
					return std::format("Value: {}", arg.type->serialize(arg.data));
				};

				if (blueprint_window.debugger->debugging == blueprint_instance)
					ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImColor(100, 80, 60, 200));
				ax::NodeEditor::Begin("node_editor");

				auto executing_object = blueprint_instance->executing_object();
					
				auto& instance_group = blueprint_instance->groups[group_name_hash];
				for (auto& b : group->blocks)
				{
					if (b->depth == 0) // skip root block
						continue;

					auto instance_object = instance_group.object_map[b->object_id];
					ax_node_editor->GetNodeBuilder();
					ax::NodeEditor::BeginNode((uint64)b.get());
					ImGui::Text("D%d", b->depth);

					b->position = ax::NodeEditor::GetNodePosition((ax::NodeEditor::NodeId)b.get());
					auto ax_node = ax_node_editor->GetNodeBuilder().m_CurrentNode;
					auto bounds = ax_node->m_GroupBounds;
					b->rect.a = bounds.Min;
					b->rect.b = bounds.Max;

					ax::NodeEditor::BeginPin((uint64)b->input.get(), ax::NodeEditor::PinKind::Input);
					ImGui::TextUnformatted((graphics::FontAtlas::icon_s("play"_h) + "  ").c_str());
					ax::NodeEditor::EndPin();

					ImGui::SameLine(0.f, max(0.f, b->rect.size().x - 56.f));

					ax::NodeEditor::BeginPin((uint64)b->output.get(), ax::NodeEditor::PinKind::Output);
					ImGui::TextUnformatted(("  " + graphics::FontAtlas::icon_s("play"_h)).c_str());
					ax::NodeEditor::EndPin();

					ax::NodeEditor::Group(vec2(200));

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

					ax::NodeEditor::BeginNode((uint64)n.get());
					auto display_name = n->name;
					if (blueprint_window.debugger->debugging == blueprint_instance && 
						executing_object && executing_object->original.p.node == n.get())
						display_name = graphics::FontAtlas::icon_s("arrow-right"_h) + " " + display_name;
					ImGui::TextUnformatted(display_name.c_str());
					ImGui::Text("D%d", n->block->depth);
					ImGui::BeginGroup();
					for (auto i = 0; i < n->inputs.size(); i++)
					{
						auto input = n->inputs[i].get();
						ax::NodeEditor::BeginPin((uint64)input, ax::NodeEditor::PinKind::Input);
						ImGui::Text("%s %s", graphics::FontAtlas::icon_s("play"_h).c_str(), input->name.c_str());
						ax::NodeEditor::EndPin();
						if (blueprint_window.debugger->debugging == blueprint_instance)
						{
							if (ImGui::IsItemHovered())
							{
								auto& arg = instance_object->inputs[i];
								if (arg.type)
								{
									tooltip = get_slot_value(arg);
									ax::NodeEditor::Suspend();
									tooltip_pos = io.MousePos;
									ax::NodeEditor::Resume();
								}
							}
						}
						else
						{
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
								auto changed = 0;
								ImGui::PushID(input);
								if (input->type && input->type->tag == TagD)
								{
									auto ti = (TypeInfo_Data*)input->type;
									switch (ti->data_type)
									{
									case DataBool:
										ImGui::SetNextItemWidth(100.f);
										changed |= ImGui::Checkbox("", (bool*)input->data);
										break;
									case DataFloat:
										ImGui::PushMultiItemsWidths(ti->vec_size, 60.f * ti->vec_size);
										for (int i = 0; i < ti->vec_size; i++)
										{
											ImGui::PushID(i);
											if (i > 0)
												ImGui::SameLine();
											ImGui::DragScalar("", ImGuiDataType_Float, &((float*)input->data)[i], 0.01f);
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
											ImGui::DragScalar("", ImGuiDataType_S32, &((int*)input->data)[i]);
											changed |= ImGui::IsItemDeactivatedAfterEdit();
											ImGui::PopID();
											ImGui::PopItemWidth();
										}
										break;
									case DataString:
										ImGui::SetNextItemWidth(100.f);
										ImGui::InputText("", (std::string*)input->data);
										changed |= ImGui::IsItemDeactivatedAfterEdit();
										break;
									case DataPath:
									{
										auto& path = *(std::filesystem::path*)input->data;
										auto s = path.string();
										ImGui::SetNextItemWidth(100.f);
										ImGui::InputText(display_name.c_str(), s.data(), ImGuiInputTextFlags_ReadOnly);
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
										if (ImGui::Button(graphics::FontAtlas::icon_s("location-crosshairs"_h).c_str()))
											project_window.ping(Path::get(path));
										ImGui::SameLine();
										if (ImGui::Button(graphics::FontAtlas::icon_s("xmark"_h).c_str()))
										{
											path = L"";
											changed = true;
										}
									}
										break;
									}
								}
								ImGui::PopID();
								if (changed)
								{
									auto frame = frames;
									input->data_changed_frame = frame;
									group->data_changed_frame = frame;
									blueprint->dirty_frame = frame;
								}
							}
						}
					}
					ImGui::EndGroup();
					ImGui::SameLine(0.f, 16.f);
					ImGui::BeginGroup();
					for (auto i = 0; i < n->outputs.size(); i++)
					{
						auto output = n->outputs[i].get();
						if (!output->type)
							continue;
						ax::NodeEditor::BeginPin((uint64)output, ax::NodeEditor::PinKind::Output);
						ImGui::Text("%s %s", output->name.c_str(), graphics::FontAtlas::icon_s("play"_h).c_str());
						ax::NodeEditor::EndPin();
						if (blueprint_window.debugger->debugging == blueprint_instance)
						{
							if (ImGui::IsItemHovered())
							{
								auto& arg = instance_object->outputs[i];
								if (arg.type)
								{
									tooltip = get_slot_value(arg);
									ax::NodeEditor::Suspend();
									tooltip_pos = io.MousePos;
									ax::NodeEditor::Resume();
								}
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
							if (blueprint_allow_type(to_slot->allowed_types, from_slot->type))
							{
								if (ax::NodeEditor::AcceptNewItem())
									blueprint->add_link(from_slot, to_slot);
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
							blueprint->remove_link(link);
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
							}
							else if (obj.type == BlueprintObjectBlock)
							{
								blueprint->remove_block(obj.p.block);
							}
						}
					}

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				ax::NodeEditor::EndDelete();

				BlueprintLinkPtr context_node = nullptr;
				BlueprintSlotPtr context_slot = nullptr;
				BlueprintLinkPtr context_link = nullptr;

				ax::NodeEditor::Suspend();
				if (ax::NodeEditor::ShowNodeContextMenu((ax::NodeEditor::NodeId*)&context_node))
				{
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
					if (ImGui::Selectable("To New Group"))
						;
					if (ImGui::Selectable("As Variable"))
						;
					if (ImGui::Selectable("As Const Variable"))
						;
					ImGui::EndPopup();
				}
				if (ImGui::BeginPopup("pin_context_menu"))
				{
					if (ImGui::Selectable("Break Links"))
						;
					if (ImGui::Selectable("Reset"))
						;
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
					static auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
					static auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");
					static auto entity_library = BlueprintNodeLibrary::get(L"universe::entity");

					static std::string filter = "";
					ImGui::InputText("Filter", &filter);

					auto show_node_templates = [&](BlueprintNodeLibraryPtr library) {
						for (auto& t : library->node_templates)
						{
							if (!filter.empty())
							{
								if (t.name.find(filter) == std::string::npos)
									continue;
							}
							uint to_slot_name = 0;
							if (new_node_link_slot)
							{
								for (auto& i : t.inputs)
								{
									if (blueprint_allow_type(i.allowed_types, new_node_link_slot->type))
									{
										to_slot_name = i.name_hash;
										break;
									}
								}
								if (!to_slot_name)
									continue;
							}
							if (ImGui::Selectable(t.name.c_str()))
							{
								auto n = blueprint->add_node(group, nullptr, t.name, t.inputs, t.outputs,
									t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
								n->position = open_popup_pos;
								ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)n, n->position);

								if (new_node_link_slot)
									blueprint->add_link(new_node_link_slot, n->find_input(to_slot_name));
							}
						}
					};
					if (!new_node_link_slot)
					{
						if (ImGui::Selectable("Block"))
						{
							auto b = blueprint->add_block(group, nullptr);
							b->position = open_popup_pos;
							b->rect = Rect(vec2(0), vec2(200));
							ax::NodeEditor::SetNodePosition((ax::NodeEditor::NodeId)b, b->position);
						}
					}
					show_node_templates(standard_library);
					show_node_templates(texture_library);
					show_node_templates(geometry_library);
					show_node_templates(entity_library);
					ImGui::EndPopup();

					if (blueprint_instance->built_frame < blueprint->dirty_frame)
						blueprint_instance->build();
				}
				else
					new_node_link_slot = nullptr;

				ax::NodeEditor::Resume();

				ax::NodeEditor::End();
				if (blueprint_window.debugger->debugging == blueprint_instance)
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
