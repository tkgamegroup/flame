#include "console_window.h"

ConsoleWindow console_window;

ConsoleView::ConsoleView() :
	View(&console_window, "Console##" + str(linearRand(0, 10000)))
{
}

ConsoleView::ConsoleView(const std::string& name) :
	View(&console_window, name)
{
}

void ConsoleView::on_draw()
{
	auto opened = ImGui::Begin(name.c_str());

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
		views.emplace_back(new ConsoleView);
}

void ConsoleWindow::open_view(const std::string& name)
{
	views.emplace_back(new ConsoleView(name));
}
