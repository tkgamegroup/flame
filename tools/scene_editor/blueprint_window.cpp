#include "blueprint_window.h"

#include <flame/foundation/blueprint.h>

BlueprintWindow blueprint_window;

BlueprintView::BlueprintView() :
	BlueprintView("Blueprint##" + str(rand()))
{
}

BlueprintView::BlueprintView(const std::string& name) :
	View(&blueprint_window, name)
{
#if USE_IMGUI_NODE_EDITOR
	ax::NodeEditor::Config im_editor_config;
	im_editor_config.SettingsFile = "";
	im_editor_config.NavigateButtonIndex = 2;
	im_editor = ax::NodeEditor::CreateEditor(&im_editor_config);
#endif
}

void BlueprintView::on_draw()
{
	bool opened = true;
	ImGui::Begin(name.c_str(), &opened);

	if (ImGui::Button("Test: Open Blueprint"))
	{
		if (blueprint)
		{
			Blueprint::release(blueprint);
			blueprint = nullptr;
		}
		if (blueprint_instance)
		{
			delete blueprint_instance;
			blueprint_instance = nullptr;
		}
		blueprint = Blueprint::get(L"asserts\\test.blueprint");
		blueprint_instance = BlueprintInstance::create(blueprint);
	}

	static auto standard_library = BlueprintNodeLibrary::get(L"standard");
	static auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

	if (blueprint)
	{
		if (blueprint_instance->built_frame < blueprint->dirty_frame)
			blueprint_instance->build();

		ImGui::SameLine();
		if (ImGui::Button("Add Node"))
			ImGui::OpenPopup("add_node");
		if (ImGui::BeginPopup("add_node"))
		{
			for (auto& t : standard_library->node_templates)
			{
				if (ImGui::Selectable(t.name.c_str()))
				{
					blueprint->add_node(nullptr, t.name, t.inputs, t.outputs,
						t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
				}
			}
			for (auto& t : geometry_library->node_templates)
			{
				if (ImGui::Selectable(t.name.c_str()))
				{
					blueprint->add_node(nullptr, t.name, t.inputs, t.outputs,
						t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
				}
			}
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Run"))
		{
			if (!blueprint_instance->executing_group)
				blueprint_instance->prepare_executing("main"_h);
			blueprint_instance->run();
		}
		ImGui::SameLine();
		if (ImGui::Button("Step"))
		{
			if (!blueprint_instance->executing_group)
			{
				blueprint_instance->prepare_executing("main"_h);
				blueprint_window.debugger->debugging = blueprint_instance;
			}
			else
			{
				BlueprintNodePtr break_node = nullptr;
				if (auto nn = blueprint_instance->current_node_ptr(); nn)
				{
					if (blueprint_window.debugger->has_break_node(nn->original))
					{
						break_node = nn->original;
						blueprint_window.debugger->remove_break_node(break_node);
					}
				}
				blueprint_window.debugger->debugging = nullptr;
				blueprint_instance->step();
				if (break_node)
					blueprint_window.debugger->add_break_node(break_node);
				if (blueprint_instance->executing_group)
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

		ax::NodeEditor::SetCurrentEditor(im_editor);
		if (blueprint_window.debugger->debugging == blueprint_instance)
			ax::NodeEditor::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImColor(100, 80, 60, 200));
		ax::NodeEditor::Begin("node_editor");

		BlueprintInstance::Node* break_node = nullptr;
		if (blueprint_window.debugger->debugging == blueprint_instance)
			break_node = blueprint_instance->current_node_ptr();

		auto group = blueprint->groups[0].get();
		for (auto& n : group->nodes)
		{
			const BlueprintInstance::Node* instance_node = nullptr;
			if (blueprint_window.debugger->debugging == blueprint_instance)
			{
				if (group->name_hash == blueprint_instance->executing_group)
					instance_node = blueprint_instance->current_group->find(n.get());
			}

			ax::NodeEditor::BeginNode((uint64)n.get());
			auto display_name = n->name;
			if (break_node && break_node->original == n.get())
				display_name = graphics::FontAtlas::icon_s("arrow-right"_h) + " " + display_name;
			ImGui::TextUnformatted(display_name.c_str());
			ImGui::BeginGroup();
			for (auto i = 0; i < n->inputs.size(); i++)
			{
				auto& input = n->inputs[i];
				ax::NodeEditor::BeginPin((uint64)&input, ax::NodeEditor::PinKind::Input);
				ImGui::Text("%s %s", graphics::FontAtlas::icon_s("play"_h).c_str(), input.name.c_str());
				ax::NodeEditor::EndPin();
				if (blueprint_window.debugger->debugging == blueprint_instance)
				{
					if (ImGui::IsItemHovered())
					{
						if (instance_node)
						{
							auto& arg = instance_node->inputs[i];
							if (arg.type)
							{
								tooltip = std::format("Value: {}", arg.type->serialize(arg.data));
								ax::NodeEditor::Suspend();
								tooltip_pos = io.MousePos;
								ax::NodeEditor::Resume();
							}
						}
					}
				}
				else
				{
					auto linked = false;
					for (auto& l : group->links)
					{
						if (l->to_slot == &input)
						{
							linked = true;
							break;
						}
					}
					if (!linked)
					{
						ImGui::PushID(&input);
						if (auto type = input.type; type && type->tag == TagD)
						{
							auto ti = (TypeInfo_Data*)type;
							switch (ti->data_type)
							{
							case DataFloat:
								ImGui::PushMultiItemsWidths(ti->vec_size, 60.f * ti->vec_size);
								for (int i = 0; i < ti->vec_size; i++)
								{
									ImGui::PushID(i);
									if (i > 0)
										ImGui::SameLine();
									ImGui::DragScalar("", ImGuiDataType_Float, &((float*)input.data)[i], 0.01f);
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
									ImGui::DragScalar("", ImGuiDataType_S32, &((int*)input.data)[i]);
									ImGui::PopID();
									ImGui::PopItemWidth();
								}
								ImGui::PopItemWidth();
								break;
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
				auto& output = n->outputs[i];
				if (!output.type)
					continue;
				ax::NodeEditor::BeginPin((uint64)&output, ax::NodeEditor::PinKind::Output);
				ImGui::Text("%s %s", output.name.c_str(), graphics::FontAtlas::icon_s("play"_h).c_str());
				ax::NodeEditor::EndPin();
				if (blueprint_window.debugger->debugging == blueprint_instance)
				{
					if (ImGui::IsItemHovered())
					{
						if (instance_node)
						{
							auto& arg = instance_node->outputs[i];
							if (arg.type)
							{
								tooltip = std::format("Value: {}", arg.type->serialize(arg.data));
								ax::NodeEditor::Suspend();
								tooltip_pos = io.MousePos;
								ax::NodeEditor::Resume();
							}
						}
					}
				}
			}
			ImGui::EndGroup();

			if (n->preview_provider)
			{
				if (ImGui::CollapsingHeader("Preview"))
				{
					ImGui::Image(nullptr, ImVec2(200, 200));
				}
			}

			ax::NodeEditor::EndNode();
		}

		for (auto& l : group->links)
			ax::NodeEditor::Link((uint64)l.get(), (uint64)l->from_slot, (uint64)l->to_slot);

		if (ax::NodeEditor::BeginCreate())
		{
			if (BlueprintSlotPtr from_slot, to_slot;
				ax::NodeEditor::QueryNewLink((ax::NodeEditor::PinId*)&from_slot, (ax::NodeEditor::PinId*)&to_slot))
			{
				if (from_slot && to_slot && from_slot != to_slot)
				{
					if (to_slot->allow_type(from_slot->type))
					{
						if (ax::NodeEditor::AcceptNewItem())
							blueprint->add_link(from_slot->node, from_slot->name_hash, to_slot->node, to_slot->name_hash);
					}
				}
			}
		}
		ax::NodeEditor::EndCreate();
		if (ax::NodeEditor::BeginDelete())
		{
			BlueprintLinkPtr link;
			BlueprintNodePtr node;
			while (ax::NodeEditor::QueryDeletedLink((ax::NodeEditor::LinkId*)&link))
			{
				if (ax::NodeEditor::AcceptDeletedItem())
					blueprint->remove_link(link);
			}
			while (ax::NodeEditor::QueryDeletedNode((ax::NodeEditor::NodeId*)&node))
			{
				if (ax::NodeEditor::AcceptDeletedItem())
					blueprint->remove_node(node);
			}
		}
		ax::NodeEditor::EndDelete();

		ax::NodeEditor::End();
		if (blueprint_window.debugger->debugging == blueprint_instance)
			ax::NodeEditor::PopStyleColor();
		ax::NodeEditor::SetCurrentEditor(nullptr);

		if (!tooltip.empty())
		{
			auto mpos = io.MousePos;
			io.MousePos = tooltip_pos;
			ImGui::SetTooltip(tooltip.c_str());
			io.MousePos = mpos;
		}
	}

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

void BlueprintWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		new BlueprintView;
}

void BlueprintWindow::open_view(const std::string& name)
{
	new BlueprintView(name);
}
