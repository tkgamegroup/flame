#include "console_window.h"

ConsoleWindow console_window;

ConsoleView::ConsoleView() :
	View(&console_window, "Console##" + str(rand()))
{
}

ConsoleView::ConsoleView(const std::string& name) :
	View(&console_window, name)
{
}

void ConsoleView::on_draw()
{
	bool opened = true;
	ImGui::Begin(name.c_str(), &opened);

	ImGui::End();
	if (!opened)
		delete this;
}

ConsoleWindow::ConsoleWindow() :
	Window("Console")
{
}

void ConsoleWindow::open_view(bool new_instance)
{
	if (new_instance || views.empty())
		new ConsoleView;
}

void ConsoleWindow::open_view(const std::string& name)
{
	new ConsoleView(name);
}
