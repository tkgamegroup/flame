#include <flame/universe/app.h>

using namespace flame;
using namespace graphics;

struct MyApp : App
{
	void create();
	void create_widgets();
}g_app;

struct MainForm : GraphicsWindow
{
	MainForm();
};

void MyApp::create()
{
	App::create();
}

void MyApp::create_widgets()
{
//	ui.e_begin_layout(LayoutVertical, 0.f, false, false);
//	ui.c_aligner(AlignMinMax, AlignMinMax);
//		ui.e_begin_menu_bar();
//			ui.e_begin_menubar_menu(L"Style");
//				ui.e_menu_item(L"Dark", [](Capture& c) {
//					looper().add_event([](Capture& c) {
//						auto& ui = main_window->ui;
//						main_window->root->remove_children(0, -1);
//						ui.style_set_to_dark();
//						main_window->canvas->clear_color = vec4(ui.style(BackgroundColor).c) / 255.f;
//						g_app.create_widgets();
//					}, Capture());
//				}, Capture());
//				ui.e_menu_item(L"Light", [](Capture& c) {
//					looper().add_event([](Capture& c) {
//						auto& ui = main_window->ui;
//						main_window->root->remove_children(0, -1);
//						ui.style_set_to_light();
//						main_window->canvas->clear_color = vec4(ui.style(BackgroundColor).c) / 255.f;
//						g_app.create_widgets();
//					}, Capture());
//				}, Capture());
//			ui.e_end_menubar_menu();
//			ui.e_begin_menubar_menu(L"Window");
//				ui.e_menu_item(L"Reflector", [](Capture& c) {
//					auto& ui = main_window->ui;
//					ui.next_element_pos = vec2(100.f);
//					create_ui_reflector(ui);
//				}, Capture());
//			ui.e_end_menubar_menu();
//		ui.e_end_menu_bar();
//
//		ui.e_begin_layout();
//		ui.c_aligner(AlignMinMax, AlignMinMax);
//			ui.next_element_pos = vec2(16.f, 10.f);
//			ui.e_begin_layout(LayoutVertical, 16.f);
//				ui.e_text(L"Text");
//				ui.e_button(L"Click Me!", [](Capture& c) {
//					c.current<cReceiver>()->entity->get_component(cText)->set_text(L"Click Me! :)");
//					printf("thank you for clicking me\n");
//				}, Capture());
//				ui.e_begin_layout(LayoutHorizontal, 4.f);
//				ui.e_text(L"Checkbox");
//				ui.e_checkbox();
//				ui.e_end_layout();
//				ui.next_element_size = 258.f;
//				ui.next_element_padding = 4.f;
//				ui.next_element_frame_thickness = 2.f;
//				ui.next_element_frame_color = ui.style(ForegroundColor).c;
//				ui.e_image(img_id << 16);
//				ui.e_edit(100.f);
//			ui.e_end_layout();
//
//			ui.next_element_pos = vec2(416.f, 10.f);
//			ui.e_begin_layout(LayoutVertical, 16.f);
//			ui.next_element_size = vec2(200.f, 100.f);
//			ui.next_element_padding = 4.f;
//			ui.next_element_frame_thickness = 2.f;
//			ui.next_element_frame_color = ui.style(ForegroundColor).c;
//				ui.e_begin_scrollbar(ScrollbarVertical, false);
//					ui.e_begin_list(true);
//						for (auto i = 0; i < 10; i++)
//							ui.e_list_item((L"item" + std::to_wstring(i)).c_str());
//					ui.e_end_list();
//				ui.e_end_scrollbar();
//
//				ui.next_element_padding = 4.f;
//				ui.next_element_frame_thickness = 2.f;
//				ui.next_element_frame_color = ui.style(ForegroundColor).c;
//				ui.e_begin_tree(false);
//					ui.e_begin_tree_node(L"A");
//						ui.e_tree_leaf(L"C");
//						ui.e_tree_leaf(L"D");
//					ui.e_end_tree_node();
//					ui.e_tree_leaf(L"B");
//				ui.e_end_tree();
//
//				ui.e_begin_combobox();
//					ui.e_combobox_item(L"Apple");
//					ui.e_combobox_item(L"Boy");
//					ui.e_combobox_item(L"Cat");
//				ui.e_end_combobox();
//			ui.e_end_layout();
//
//		ui.e_end_layout();
//
//	ui.e_end_layout();
//
//	ui.next_element_pos = vec2(416.f, 300.f);
//	ui.next_element_size = vec2(200.f, 200.f);
//	ui.e_begin_docker_floating_container();
//		ui.e_begin_docker();
//			ui.e_begin_docker_page(L"ResourceExplorer").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
//				ui.e_text(L"flower.png  main.cpp");
//			ui.e_end_docker_page();
//		ui.e_end_docker();
//	ui.e_end_docker_floating_container();
//
//	ui.next_element_pos = vec2(640.f, 300.f);
//	ui.next_element_size = vec2(200.f, 200.f);
//	ui.e_begin_docker_floating_container();
//		ui.e_begin_docker_layout(LayoutHorizontal);
//			ui.e_begin_docker();
//				ui.e_begin_docker_page(L"TextEditor").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
//					ui.e_text(L"printf(\"Hello World!\\n\");");
//				ui.e_end_docker_page();
//			ui.e_end_docker();
//			ui.e_begin_docker_layout(LayoutVertical);
//				ui.e_begin_docker();
//					ui.e_begin_docker_page(L"Hierarchy").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
//						ui.e_text(L"Node A\n--Node B");
//					ui.e_end_docker_page();
//				ui.e_end_docker();
//				ui.e_begin_docker();
//					ui.e_begin_docker_page(L"Inspector").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
//						ui.e_text(L"Name: James Bond\nID: 007");
//					ui.e_end_docker_page();
//				ui.e_end_docker();
//			ui.e_end_docker_layout();
//		ui.e_end_docker_layout();
//	ui.e_end_docker_floating_container();
//
//	{
//		auto menu = root->get_component(cMenu);
//		if (menu)
//			root->remove_component(menu);
//	}
//	ui.e_begin_popup_menu();
//		ui.e_menu_item(L"Refresh", [](Capture& c) {
//			wprintf(L"%s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//		}, Capture());
//		ui.e_menu_item(L"Save", [](Capture& c) {
//			wprintf(L"%s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//		}, Capture());
//		ui.e_menu_item(L"Help", [](Capture& c) {
//			wprintf(L"%s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//		}, Capture());
//		ui.e_separator();
//		ui.e_begin_sub_menu(L"Add");
//			ui.e_menu_item(L"Tree", [](Capture& c) {
//				wprintf(L"Add %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//			ui.e_menu_item(L"Car", [](Capture& c) {
//				wprintf(L"Add %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//			ui.e_menu_item(L"House", [](Capture& c) {
//				wprintf(L"Add %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//		ui.e_end_sub_menu();
//		ui.e_begin_sub_menu(L"Remove");
//			ui.e_menu_item(L"Tree", [](Capture& c) {
//				wprintf(L"Remove %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//			ui.e_menu_item(L"Car", [](Capture& c) {
//				wprintf(L"Remove %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//			ui.e_menu_item(L"House", [](Capture& c) {
//				wprintf(L"Remove %s!\n", c.current<cReceiver>()->entity->get_component(cText)->text.v);
//			}, Capture());
//		ui.e_end_sub_menu();
//	ui.e_end_popup_menu();
}

MainForm::MainForm() :
	GraphicsWindow(&g_app, L"UI Test", uvec2(1280, 720), WindowFrame | WindowResizable)
{
	//canvas->set_resource(-1, Image::create_from_file(g_app.graphics_device, (g_app.engine_path / L"assets/9.png").c_str())->default_view());

	g_app.create_widgets();
}

int main(int argc, char** args)
{
	g_app.create();
	new MainForm();
	{
		auto e = Entity::create();
		e->load(L"ui_test");
		g_app.main_window->root->add_child(e);
	}
	g_app.run();

	return 0;
}
