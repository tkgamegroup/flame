#include <flame/utils/app.h>
#include <flame/utils/fps.h>
#include <flame/universe/ui/reflector.h>

using namespace flame;
using namespace graphics;

struct MainForm : GraphicsWindow
{
	UI ui;

	MainForm();
	~MainForm();
};

MainForm* main_window;

struct MyApp : App
{
}app;

MainForm::MainForm() :
	GraphicsWindow(&app, true, true, "Schedule Test", Vec2u(1280, 720), WindowFrame | WindowResizable)
{
	main_window = this;

	setup_as_main_window();

	ui.init(world);
	
	ui.parents.push(root);

	ui.e_begin_menu_bar();
		ui.e_begin_menubar_menu(L"Window");
			ui.e_menu_item(L"Reflector", [](Capture& c) {
				auto& ui = main_window->ui;
				ui.next_element_pos = Vec2f(100.f);
				create_ui_reflector(ui);
			}, Capture());
		ui.e_end_menubar_menu();
	ui.e_end_menu_bar();

	ui.e_text(L"");
	ui.c_aligner(AlignMin, AlignMax);
	looper().add_event([](Capture& c) {
		c.thiz<cText>()->set_text(std::to_wstring(fps).c_str());
		c._current = nullptr;
	}, Capture().set_thiz(e->get_component<cText>()), 1.f);

	ui.next_element_pos = Vec2f(100.f);
	ui.next_element_size = Vec2f(100.f);
	ui.next_element_color = Vec4c(255);
	auto schdule = Schedule::create();
	auto element = ui.e_element()->get_component(cElement);
	element->pivot = 0.5f;
	schdule->begin_group();
	element->scale_to(schdule, 0.f, 5.f, 3.f);
	element->move_to(schdule, 3.f, 10.f, Vec2f(500.f));
	schdule->end_group();
	element->move_to(schdule, 0.f, 3.f, Vec2f(1000.f, 100.f));
	schdule->start();

	ui.parents.pop();
}

MainForm::~MainForm()
{
	main_window = nullptr;
}

int main(int argc, char** args)
{
	app.create();
	new MainForm();

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
