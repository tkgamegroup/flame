#include "debugger_window.h"

#include <flame/graphics/image.h>
#include <flame/graphics/debug.h>

DebuggerWindow debugger_window;

DebuggerView::DebuggerView() :
	DebuggerView(debugger_window.views.empty() ? "Debugger" : "Debugger##" + str(rand()))
{
}

DebuggerView::DebuggerView(const std::string& name) :
	View(&debugger_window, name)
{
}

void DebuggerView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);
	imgui_window = ImGui::GetCurrentWindow();

	title_context_menu();

	if (ImGui::BeginTable("main", 2, ImGuiTableFlags_Resizable))
	{
		ImGui::TableSetupColumn("left", ImGuiTableColumnFlags_WidthFixed, 200.f);
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);

		auto found_last_image = false;
		if (ImGui::TreeNode("Images"))
		{
			auto all_images = graphics::Debug::get_images();
			for (auto i : all_images)
			{
				auto display_name = std::format("{} {}", i->filename.string(), str(i));
				if (ImGui::Selectable(display_name.c_str(), i == selected_image))
					selected_image = i;
				if (i == selected_image)
					found_last_image = true;
			}
			ImGui::TreePop();
		}
		if (!found_last_image)
			selected_image = nullptr;

		ImGui::TableSetColumnIndex(1);
		if (selected_image)
			view_image(selected_image, &view_swizzle, &view_sampler, &view_level, &view_layer, &view_zoom, &view_range_min, &view_range_max);

		ImGui::EndTable();
	}

	ImGui::End();
	if (!opened)
		delete this;
}

DebuggerWindow::DebuggerWindow() :
	Window("Debugger")
{
}

View* DebuggerWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new DebuggerView;
	return nullptr;
}

View* DebuggerWindow::open_view(const std::string& name)
{
	return new DebuggerView(name);
}
