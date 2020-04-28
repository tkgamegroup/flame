#include <flame/utils/app.h>
#include <flame/utils/fps.h>
#include <flame/universe/utils/ui_reflector.h>

#include <flame/universe/utils/ui_impl.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
}app;

struct MainWindow : App::Window
{
	MainWindow();
};

MainWindow* main_window;

MainWindow::MainWindow() :
	App::Window(&app, true, true, "UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable)
{
	main_window = this;

	setup_as_main_window();

	utils::set_current_root(root);
	utils::set_current_entity(root);
	
	utils::push_parent(root);

	utils::e_begin_menu_bar();
		utils::e_begin_menubar_menu(L"Window");
			utils::e_menu_item(L"Reflector", [](Capture& c) {
				utils::next_element_pos = Vec2f(100.f);
				utils::e_ui_reflector_window();
			}, Capture());
		utils::e_end_menubar_menu();
	utils::e_end_menu_bar();

	utils::e_text(L"");
	utils::c_aligner(AlignMin, AlignMax);
	add_fps_listener([](Capture& c, uint fps) {
		c.thiz<cText>()->set_text(std::to_wstring(fps).c_str());
	}, Capture().set_thiz(utils::current_entity()->get_component(cText)));

	utils::next_element_pos = Vec2f(16.f, 28.f);
	utils::e_begin_layout(LayoutVertical, 16.f);
		auto e_scene = utils::e_element();
		{
			auto c_element = e_scene->get_component(cElement);
			c_element->size = 500.f;
			c_element->frame_thickness = 2.f;
		}

		utils::e_begin_layout(LayoutHorizontal, 4.f);
			utils::e_button(L"Create Sample Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					auto e_scene = c.thiz<Entity>();
					e_scene->remove_children(0, -1);

					auto e_box1 = Entity::create();
					e_scene->add_child(e_box1);
					{
						auto c_element = cElement::create();
						c_element->pos = 50.f;
						c_element->size = 400.f;
						c_element->color = Vec4c(255, 0, 0, 255);
						e_box1->add_component(c_element);
					}

					auto e_box2 = Entity::create();
					e_box1->add_child(e_box2);
					{
						auto c_element = cElement::create();
						c_element->pos = 50.f;
						c_element->size = 300.f;
						c_element->color = Vec4c(255, 255, 0, 255);
						e_box2->add_component(c_element);
					}

					auto e_text = Entity::create();
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
			utils::e_button(L"Clear Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					c.thiz<Entity>()->remove_children(0, -1);
				}, Capture().set_thiz(c.thiz<Entity>()));
			}, Capture().set_thiz(e_scene));
			utils::e_button(L"Save Scene", [](Capture& c) {
				auto e_scene = c.thiz<Entity>();
				if (e_scene->child_count() > 0)
					Entity::save_to_file(e_scene->child(0), L"test.prefab");
			}, Capture().set_thiz(e_scene));
			utils::e_button(L"Load Scene", [](Capture& c) {
				looper().add_event([](Capture& c) {
					auto e_scene = c.thiz<Entity>();
					e_scene->remove_children(0, -1);
					if (std::filesystem::exists(L"test.prefab"))
						e_scene->add_child(Entity::create_from_file(e_scene->world(), L"test.prefab"));
				}, Capture().set_thiz(c.thiz<Entity>()));
			}, Capture().set_thiz(e_scene));
		utils::e_end_layout();
	utils::e_end_layout();

	utils::pop_parent();
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
