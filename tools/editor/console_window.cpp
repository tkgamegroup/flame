#include "console_window.h"

ConsoleWindow console_window;

ConsoleView::ConsoleView() :
	ConsoleView(console_window.views.empty() ? "Console" : "Console##" + str(rand()))
{
}

ConsoleView::ConsoleView(const std::string& name) :
	View(&console_window, name)
{
}

void ConsoleView::on_draw()
{
	bool opened = true;
	ImGui::SetNextWindowSize(vec2(400, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin(name.c_str(), &opened);
	imgui_window = ImGui::GetCurrentWindow();

	ImGui::End();
	if (!opened)
		delete this;
}

ConsoleWindow::ConsoleWindow() :
	Window("Console")
{
}

View* ConsoleWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		return new ConsoleView;
	return nullptr;
}

View* ConsoleWindow::open_view(const std::string& name)
{
	return new ConsoleView(name);
}
