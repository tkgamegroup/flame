#include "blueprint_window.h"

#include <flame/foundation/blueprint.h>

BlueprintWindow blueprint_window;

BlueprintView::BlueprintView() :
	View(&blueprint_window, "Blueprint##" + str(rand()))
{
}

BlueprintView::BlueprintView(const std::string& name) :
	View(&blueprint_window, name)
{
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
		blueprint = Blueprint::get(L"asserts\\test.blueprint");
	}

	if (blueprint)
	{
		ImGui::SameLine();
		if (ImGui::Button("Test: Add Node"))
		{
			auto n = blueprint->add_node(nullptr, "Add");
			n->position = ImGui::GetCursorPos();
		}

		ax::NodeEditor::SetCurrentEditor(blueprint_window.im_editor);
		ax::NodeEditor::Begin("node_editor");

		auto& io = ImGui::GetIO();
		auto dl = ImGui::GetWindowDrawList();

		auto group = blueprint->groups[0].get();
		for (auto& n : group->nodes)
		{
			ax::NodeEditor::BeginNode((uint64)n.get());
			ImGui::TextUnformatted(n->name.c_str());
			for (auto& i : n->inputs)
			{
				ax::NodeEditor::BeginPin((uint64)&i, ax::NodeEditor::PinKind::Input);
				ImGui::Text("%s %s", graphics::FontAtlas::icon_s("play"_h).c_str(), i.name.c_str());
				ax::NodeEditor::EndPin();
			}
			ImGui::SameLine();
			for (auto& o : n->outputs)
			{
				ax::NodeEditor::BeginPin((uint64)&o, ax::NodeEditor::PinKind::Output);
				ImGui::Text("%s %s", o.name.c_str(), graphics::FontAtlas::icon_s("play"_h).c_str());
				ax::NodeEditor::EndPin();
			}
			ax::NodeEditor::EndNode();
		}
		for (auto& l : group->links)
			ax::NodeEditor::Link((uint64)l.get(), (uint64)l->from_slot, (uint64)l->to_slot);

		if (ax::NodeEditor::BeginCreate())
		{
			BlueprintSlot* from_slot; BlueprintSlot* to_slot;
			if (ax::NodeEditor::QueryNewLink((ax::NodeEditor::PinId*)&from_slot, (ax::NodeEditor::PinId*)&to_slot))
			{
				if (from_slot && to_slot)
				{
					if (ax::NodeEditor::AcceptNewItem())
					{

					}
				}
			}
		}
		ax::NodeEditor::EndCreate();

		ax::NodeEditor::End();
		ax::NodeEditor::SetCurrentEditor(nullptr);
	}

	ImGui::End();
	if (!opened)
		delete this;
}

BlueprintWindow::BlueprintWindow() :
	Window("Blueprint")
{
#if USE_IMGUI_NODE_EDITOR
	im_editor = ax::NodeEditor::CreateEditor();
#endif
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
