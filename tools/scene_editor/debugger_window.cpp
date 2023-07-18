#include "debugger_window.h"

#include <flame/graphics/image.h>
#include <flame/graphics/debug.h>

DebuggerWindow debugger_window;

DebuggerView::DebuggerView() :
	View(&debugger_window, "Debugger##" + str(linearRand(0, 10000)))
{
}

DebuggerView::DebuggerView(const std::string& name) :
	View(&debugger_window, name)
{
}

void DebuggerView::on_draw()
{
	auto opened = ImGui::Begin(name.c_str());

	if (ImGui::TreeNode("Images"))
	{
		auto all_images = graphics::Debug::get_images();
		for (auto i : all_images)
		{
			if (ImGui::TreeNode(std::format("{} {} {}", str(i), TypeInfo::serialize_t(i->format), str(i->extent)).c_str()))
			{
				ImGui::Image(i, (vec2)i->extent);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}

	ImGui::End();
	if (!opened)
		delete this;
}

DebuggerWindow::DebuggerWindow() :
	Window("Debugger")
{
}

void DebuggerWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		views.emplace_back(new DebuggerView);
}

void DebuggerWindow::open_view(const std::string& name)
{
	views.emplace_back(new DebuggerView(name));
}
