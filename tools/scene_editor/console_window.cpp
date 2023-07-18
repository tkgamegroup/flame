#include "console_window.h"

ConsoleWindow console_window;

ConsoleView::ConsoleView() :
	View("Console##" + str(linearRand(0, 10000)))
{
}

ConsoleView::ConsoleView(const std::string& name) :
	View(name)
{
}

ConsoleView::~ConsoleView()
{
	std::erase_if(console_window.views, [&](const auto& i) {
		return i.get() == this;
	});
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
