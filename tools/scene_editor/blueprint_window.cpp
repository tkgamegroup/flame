#include "blueprint_window.h"

#include <flame/foundation/blueprint.h>
#include <flame/graphics/model.h>
#include <flame/universe/components/node.h>
#include <flame/universe/components/camera.h>
#include <flame/universe/components/mesh.h>

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

struct BpNodePreview
{
	ModelPreviewer model_previewer;

	~BpNodePreview()
	{
		model_previewer.destroy();
	}
};
static std::map<BlueprintNodePtr, BpNodePreview> previews;

void BlueprintView::close_blueprint()
{
	if (blueprint)
	{
		previews.clear();
		Blueprint::release(blueprint);
		blueprint = nullptr;
	}
	if (blueprint_instance)
	{
		delete blueprint_instance;
		blueprint_instance = nullptr;
	}
}

void BlueprintView::on_draw()
{
	bool opened = true;
	ImGui::Begin(name.c_str(), &opened);

	if (ImGui::Button("Test: Open Blueprint"))
	{
		close_blueprint();
		blueprint = Blueprint::get(L"asserts\\test.blueprint");
		blueprint_instance = BlueprintInstance::create(blueprint);
	}
	ImGui::SameLine();
	if (ImGui::Button("Close Blueprint"))
	{
		close_blueprint();
	}

	static uint group_name = "main"_h;

	if (blueprint)
	{
		if (blueprint_instance->built_frame < blueprint->dirty_frame)
			blueprint_instance->build();

		static auto standard_library = BlueprintNodeLibrary::get(L"standard");
		static auto texture_library = BlueprintNodeLibrary::get(L"graphics::texture");
		static auto geometry_library = BlueprintNodeLibrary::get(L"graphics::geometry");

		if (ImGui::Button("Add Node"))
			ImGui::OpenPopup("add_node");
		if (ImGui::BeginPopup("add_node"))
		{
			auto show_node_templates = [&](BlueprintNodeLibraryPtr library) {
				for (auto& t : library->node_templates)
				{
					if (ImGui::Selectable(t.name.c_str()))
					{
						blueprint->add_node(nullptr, t.name, t.inputs, t.outputs,
							t.function, t.constructor, t.destructor, t.input_slot_changed_callback, t.preview_provider);
					}
				}
			};
			show_node_templates(standard_library);
			show_node_templates(texture_library);
			show_node_templates(geometry_library);
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
		auto get_slot_value = [](BlueprintArgument& arg)->std::string {
			if (arg.type->tag != TagD)
				return "";
			return std::format("Value: {}", arg.type->serialize(arg.data));
		};

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
			BlueprintInstance::Node* instance_node = nullptr;
			instance_node = (BlueprintInstance::Node*)blueprint_instance->groups[group_name].find(n.get());

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
								tooltip = get_slot_value(arg);
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
						auto changed = 0;
						ImGui::PushID(&input);
						if (input.type && input.type->tag == TagD)
						{
							auto ti = (TypeInfo_Data*)input.type;
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
									ImGui::DragScalar("", ImGuiDataType_S32, &((int*)input.data)[i]);
									changed |= ImGui::IsItemDeactivatedAfterEdit();
									ImGui::PopID();
									ImGui::PopItemWidth();
								}
								break;
							}
						}
						ImGui::PopID();
						if (changed)
						{
							auto frame = frames;
							input.data_changed_frame = frame;
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
								tooltip = get_slot_value(arg);
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
				if (instance_node)
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
				{
					if (auto it = previews.find(node); it != previews.end())
						previews.erase(it);
					blueprint->remove_node(node);
				}
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
