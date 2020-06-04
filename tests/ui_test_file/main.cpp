#include <flame/utils/app.h>
#include <flame/utils/fps.h>
#include <flame/universe/utils/ui_reflector.h>

using namespace flame;
using namespace graphics;

struct MainWindow : App::Window
{
	UI ui;

	MainWindow();
	~MainWindow();
};

MainWindow* main_window;

struct MyApp : App
{
}app;

MainWindow::MainWindow() :
	App::Window(&app, true, true, "UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable)
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
	add_fps_listener([](Capture& c, uint fps) {
		c.thiz<cText>()->set_text(std::to_wstring(fps).c_str());
	}, Capture().set_thiz(ui.current_entity->get_component(cText)));

	ui.next_element_pos = Vec2f(16.f, 28.f);
	ui.e_begin_layout(LayoutVertical, 16.f);
		auto e_scene = ui.e_element();
		{
			auto c_element = e_scene->get_component(cElement);
			c_element->size = 500.f;
			c_element->frame_thickness = 2.f;
		}

		ui.e_begin_layout(LayoutHorizontal, 4.f);
			ui.e_button(L"Create Sample Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					auto e_scene = c.thiz<Entity>();
					e_scene->remove_children(0, -1);

					auto e_box1 = f_new<Entity>();
					e_scene->add_child(e_box1);
					{
						auto c_element = cElement::create();
						c_element->pos = 50.f;
						c_element->size = 400.f;
						c_element->color = Vec4c(255, 0, 0, 255);
						e_box1->add_component(c_element);
					}

					auto e_box2 = f_new<Entity>();
					e_box1->add_child(e_box2);
					{
						auto c_element = cElement::create();
						c_element->pos = 50.f;
						c_element->size = 300.f;
						c_element->color = Vec4c(255, 255, 0, 255);
						e_box2->add_component(c_element);
					}

					auto e_text = f_new<Entity>();
					e_box2->add_child(e_text);
					{
						auto c_element = cElement::create();
						c_element->pos.x() = 12.f;
						c_element->pos.y() = 8.f;
						e_text->add_component(c_element);

						auto c_text = cText::create();
						c_text->font_atlas = app.font_atlas;
						c_text->font_size = 16;
						c_text->color = Vec4c(0, 0, 0, 255);
						c_text->set_text(L"Hello World!");
						e_text->add_component(c_text);
					}

				}, Capture().set_thiz(c.thiz<Entity>()));
			}, Capture().set_thiz(e_scene));
			ui.e_button(L"Clear Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					c.thiz<Entity>()->remove_children(0, -1);
				}, Capture().set_thiz(c.thiz<Entity>()));
			}, Capture().set_thiz(e_scene));
			ui.e_button(L"Save Scene", [](Capture& c) {
				auto e_scene = c.thiz<Entity>();
				if (e_scene->children.s > 0)
					Entity::save_to_file(e_scene->children[0], L"test.prefab");
			}, Capture().set_thiz(e_scene));
			ui.e_button(L"Load Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					auto e_scene = c.thiz<Entity>();
					e_scene->remove_children(0, -1);
					if (std::filesystem::exists(L"test.prefab"))
						e_scene->add_child(Entity::create_from_file(e_scene->world, L"test.prefab"));
				}, Capture().set_thiz(c.thiz<Entity>()));
			}, Capture().set_thiz(e_scene));
		ui.e_end_layout();
	ui.e_end_layout();

	ui.parents.pop();
}

MainWindow::~MainWindow()
{
	main_window = nullptr;
}

int main(int argc, char** args)
{
	app.create();
	new MainWindow();

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
