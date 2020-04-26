#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>
#include <flame/universe/utils/ui_reflector.h>

#include <flame/universe/utils/ui_impl.h>

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct MyApp : App
{
}app;

struct MainWindow : App::Window
{
	MainWindow() :
		App::Window(&app, true, true, "UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable)
	{
		setup_as_main_window();

		utils::set_current_root(root);
		utils::set_current_entity(root);

		canvas->clear_color = Vec4f(utils::style(BackgroundColor).c) / 255.f;
		canvas->set_resource(img_id, Imageview::create(Image::create_from_file(app.graphics_device, (app.engine_path / L"art/9.png").c_str())));

		create_widgets();
	}

	void create_widgets()
	{
		utils::push_parent(root);

		utils::e_begin_layout(LayoutVertical, 0.f, false, false);
		utils::c_aligner(AlignMinMax, AlignMinMax);
		utils::e_begin_menu_bar();
		utils::e_begin_menubar_menu(L"Style");
		utils::e_menu_item(L"Dark", [](Capture& c) {
			looper().add_event([](Capture& c) {
				auto thiz = c.thiz<MainWindow>();
				thiz->root->remove_children(0, -1);
				utils::style_set_to_dark();
				thiz->canvas->clear_color = Vec4f(utils::style(BackgroundColor).c) / 255.f;
				thiz->create_widgets();
			}, Capture().set_thiz(c._thiz));
		}, Capture().set_thiz(this));
		utils::e_menu_item(L"Light", [](Capture& c) {
			looper().add_event([](Capture& c) {
				auto thiz = c.thiz<MainWindow>();
				thiz->root->remove_children(0, -1);
				utils::style_set_to_light();
				thiz->canvas->clear_color = Vec4f(utils::style(BackgroundColor).c) / 255.f;
				thiz->create_widgets();
			}, Capture().set_thiz(c._thiz));
		}, Capture().set_thiz(this));
		utils::e_end_menubar_menu();
		utils::e_begin_menubar_menu(L"Window");
		utils::e_menu_item(L"Reflector", [](Capture& c) {
			utils::next_element_pos = Vec2f(100.f);
			utils::e_ui_reflector_window();
		}, Capture());
		utils::e_end_menubar_menu();
		utils::e_end_menu_bar();

		utils::e_begin_layout();
		utils::c_aligner(AlignMinMax, AlignMinMax);
		utils::next_element_pos = Vec2f(16.f, 10.f);
		utils::e_begin_layout(LayoutVertical, 16.f);
		utils::e_text(L"Text");
		utils::e_button(L"Click Me!", [](Capture& c) {
			c.current<cEventReceiver>()->entity->get_component(cText)->set_text(L"Click Me! :)");
			printf("thank you for clicking me\n");
		}, Capture());
		utils::e_checkbox(L"Checkbox");
		utils::next_element_size = 258.f;
		utils::next_element_padding = 4.f;
		utils::next_element_frame_thickness = 2.f;
		utils::next_element_frame_color = utils::style(ForegroundColor).c;
		utils::e_image(img_id << 16);
		utils::e_edit(100.f);
		utils::e_end_layout();

		utils::next_element_pos = Vec2f(416.f, 10.f);
		utils::e_begin_layout(LayoutVertical, 16.f);
		utils::next_element_size = Vec2f(200.f, 100.f);
		utils::next_element_padding = 4.f;
		utils::next_element_frame_thickness = 2.f;
		utils::next_element_frame_color = utils::style(ForegroundColor).c;
		utils::e_begin_scrollbar(ScrollbarVertical, false);
		utils::e_begin_list(true);
		for (auto i = 0; i < 10; i++)
			utils::e_list_item((L"item" + std::to_wstring(i)).c_str());
		utils::e_end_list();
		utils::e_end_scrollbar();

		utils::next_element_padding = 4.f;
		utils::next_element_frame_thickness = 2.f;
		utils::next_element_frame_color = utils::style(ForegroundColor).c;
		utils::e_begin_tree(false);
		utils::e_begin_tree_node(L"A");
		utils::e_tree_leaf(L"C");
		utils::e_tree_leaf(L"D");
		utils::e_end_tree_node();
		utils::e_tree_leaf(L"B");
		utils::e_end_tree();

		utils::e_begin_combobox();
		utils::e_combobox_item(L"Apple");
		utils::e_combobox_item(L"Boy");
		utils::e_combobox_item(L"Cat");
		utils::e_end_combobox();
		utils::e_end_layout();

		utils::e_end_layout();

		utils::e_end_layout();

		utils::next_element_pos = Vec2f(416.f, 300.f);
		utils::next_element_size = Vec2f(200.f, 200.f);
		utils::e_begin_docker_floating_container();
		utils::e_begin_docker();
		utils::e_begin_docker_page(L"ResourceExplorer").second->get_component(cElement)->color = utils::style(FrameColorNormal).c;
		utils::e_text(L"flower.png  main.cpp");
		utils::e_end_docker_page();
		utils::e_end_docker();
		utils::e_end_docker_floating_container();

		utils::next_element_pos = Vec2f(640.f, 300.f);
		utils::next_element_size = Vec2f(200.f, 200.f);
		utils::e_begin_docker_floating_container();
		utils::e_begin_docker_layout(LayoutHorizontal);
		utils::e_begin_docker();
		utils::e_begin_docker_page(L"TextEditor").second->get_component(cElement)->color = utils::style(FrameColorNormal).c;
		utils::e_text(L"printf(\"Hello World!\\n\");");
		utils::e_end_docker_page();
		utils::e_end_docker();
		utils::e_begin_docker_layout(LayoutVertical);
		utils::e_begin_docker();
		utils::e_begin_docker_page(L"Hierarchy").second->get_component(cElement)->color = utils::style(FrameColorNormal).c;
		utils::e_text(L"Node A\n--Node B");
		utils::e_end_docker_page();
		utils::e_end_docker();
		utils::e_begin_docker();
		utils::e_begin_docker_page(L"Inspector").second->get_component(cElement)->color = utils::style(FrameColorNormal).c;
		utils::e_text(L"Name: James Bond\nID: 007");
		utils::e_end_docker_page();
		utils::e_end_docker();
		utils::e_end_docker_layout();
		utils::e_end_docker_layout();
		utils::e_end_docker_floating_container();

		{
			auto menu = root->get_component(cMenu);
			if (menu)
				root->remove_component(menu);
		}
		utils::e_begin_popup_menu();
		utils::e_menu_item(L"Refresh", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"Save", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"Help", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_separator();
		utils::e_begin_sub_menu(L"Add");
		utils::e_menu_item(L"Tree", [](Capture& c) {
			wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"Car", [](Capture& c) {
			wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"House", [](Capture& c) {
			wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_end_sub_menu();
		utils::e_begin_sub_menu(L"Remove");
		utils::e_menu_item(L"Tree", [](Capture& c) {
			wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"Car", [](Capture& c) {
			wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_menu_item(L"House", [](Capture& c) {
			wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		utils::e_end_sub_menu();
		utils::e_end_popup_menu();

		utils::pop_parent();
	}
};

MainWindow* window = nullptr;

int main(int argc, char** args)
{
	app.create();
	window = new MainWindow();

	looper().loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
