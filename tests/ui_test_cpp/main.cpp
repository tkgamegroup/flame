#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>
#include <flame/universe/ui/ui.h>
#include <flame/universe/ui/reflector.h>

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct MyApp : App
{
	void create();
	void create_widgets();
}app;

struct MainForm : GraphicsWindow
{
	UI ui;

	MainForm();
	~MainForm() override;
	void on_update() override;
};

MainForm* main_window = nullptr;

void MyApp::create()
{
	App::create();
}

void MyApp::create_widgets()
{
	auto root = main_window->root;
	auto& ui = main_window->ui;

	ui.parents.push(root);

	ui.e_begin_layout(LayoutVertical, 0.f, false, false);
	ui.c_aligner(AlignMinMax, AlignMinMax);
		ui.e_begin_menu_bar();
			ui.e_begin_menubar_menu(L"Style");
				ui.e_menu_item(L"Dark", [](Capture& c) {
					get_looper()->add_event([](Capture& c) {
						auto& ui = main_window->ui;
						main_window->root->remove_children(0, -1);
						ui.style_set_to_dark();
						main_window->canvas->clear_color = Vec4f(ui.style(BackgroundColor).c) / 255.f;
						app.create_widgets();
					}, Capture());
				}, Capture());
				ui.e_menu_item(L"Light", [](Capture& c) {
					get_looper()->add_event([](Capture& c) {
						auto& ui = main_window->ui;
						main_window->root->remove_children(0, -1);
						ui.style_set_to_light();
						main_window->canvas->clear_color = Vec4f(ui.style(BackgroundColor).c) / 255.f;
						app.create_widgets();
					}, Capture());
				}, Capture());
			ui.e_end_menubar_menu();
			ui.e_begin_menubar_menu(L"Window");
				ui.e_menu_item(L"Reflector", [](Capture& c) {
					auto& ui = main_window->ui;
					ui.next_element_pos = Vec2f(100.f);
					create_ui_reflector(ui);
				}, Capture());
			ui.e_end_menubar_menu();
		ui.e_end_menu_bar();

		ui.e_begin_layout();
		ui.c_aligner(AlignMinMax, AlignMinMax);
			ui.next_element_pos = Vec2f(16.f, 10.f);
			ui.e_begin_layout(LayoutVertical, 16.f);
				ui.e_text(L"Text");
				ui.e_button(L"Click Me!", [](Capture& c) {
					c.current<cEventReceiver>()->entity->get_component(cText)->set_text(L"Click Me! :)");
					printf("thank you for clicking me\n");
				}, Capture());
				ui.e_begin_layout(LayoutHorizontal, 4.f);
				ui.e_text(L"Checkbox");
				ui.e_checkbox();
				ui.e_end_layout();
				ui.next_element_size = 258.f;
				ui.next_element_padding = 4.f;
				ui.next_element_frame_thickness = 2.f;
				ui.next_element_frame_color = ui.style(ForegroundColor).c;
				ui.e_image(img_id << 16);
				ui.e_edit(100.f);
			ui.e_end_layout();

			ui.next_element_pos = Vec2f(416.f, 10.f);
			ui.e_begin_layout(LayoutVertical, 16.f);
			ui.next_element_size = Vec2f(200.f, 100.f);
			ui.next_element_padding = 4.f;
			ui.next_element_frame_thickness = 2.f;
			ui.next_element_frame_color = ui.style(ForegroundColor).c;
				ui.e_begin_scrollbar(ScrollbarVertical, false);
					ui.e_begin_list(true);
						for (auto i = 0; i < 10; i++)
							ui.e_list_item((L"item" + std::to_wstring(i)).c_str());
					ui.e_end_list();
				ui.e_end_scrollbar();

				ui.next_element_padding = 4.f;
				ui.next_element_frame_thickness = 2.f;
				ui.next_element_frame_color = ui.style(ForegroundColor).c;
				ui.e_begin_tree(false);
					ui.e_begin_tree_node(L"A");
						ui.e_tree_leaf(L"C");
						ui.e_tree_leaf(L"D");
					ui.e_end_tree_node();
					ui.e_tree_leaf(L"B");
				ui.e_end_tree();

				ui.e_begin_combobox();
					ui.e_combobox_item(L"Apple");
					ui.e_combobox_item(L"Boy");
					ui.e_combobox_item(L"Cat");
				ui.e_end_combobox();
			ui.e_end_layout();

		ui.e_end_layout();

	ui.e_end_layout();

	ui.next_element_pos = Vec2f(416.f, 300.f);
	ui.next_element_size = Vec2f(200.f, 200.f);
	ui.e_begin_docker_floating_container();
		ui.e_begin_docker();
			ui.e_begin_docker_page(L"ResourceExplorer").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
				ui.e_text(L"flower.png  main.cpp");
			ui.e_end_docker_page();
		ui.e_end_docker();
	ui.e_end_docker_floating_container();

	ui.next_element_pos = Vec2f(640.f, 300.f);
	ui.next_element_size = Vec2f(200.f, 200.f);
	ui.e_begin_docker_floating_container();
		ui.e_begin_docker_layout(LayoutHorizontal);
			ui.e_begin_docker();
				ui.e_begin_docker_page(L"TextEditor").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
					ui.e_text(L"printf(\"Hello World!\\n\");");
				ui.e_end_docker_page();
			ui.e_end_docker();
			ui.e_begin_docker_layout(LayoutVertical);
				ui.e_begin_docker();
					ui.e_begin_docker_page(L"Hierarchy").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
						ui.e_text(L"Node A\n--Node B");
					ui.e_end_docker_page();
				ui.e_end_docker();
				ui.e_begin_docker();
					ui.e_begin_docker_page(L"Inspector").second->get_component(cElement)->color = ui.style(FrameColorNormal).c;
						ui.e_text(L"Name: James Bond\nID: 007");
					ui.e_end_docker_page();
				ui.e_end_docker();
			ui.e_end_docker_layout();
		ui.e_end_docker_layout();
	ui.e_end_docker_floating_container();

	{
		auto menu = root->get_component(cMenu);
		if (menu)
			root->remove_component(menu);
	}
	ui.e_begin_popup_menu();
		ui.e_menu_item(L"Refresh", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		ui.e_menu_item(L"Save", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		ui.e_menu_item(L"Help", [](Capture& c) {
			wprintf(L"%s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
		}, Capture());
		ui.e_separator();
		ui.e_begin_sub_menu(L"Add");
			ui.e_menu_item(L"Tree", [](Capture& c) {
				wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
			ui.e_menu_item(L"Car", [](Capture& c) {
				wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
			ui.e_menu_item(L"House", [](Capture& c) {
				wprintf(L"Add %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
		ui.e_end_sub_menu();
		ui.e_begin_sub_menu(L"Remove");
			ui.e_menu_item(L"Tree", [](Capture& c) {
				wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
			ui.e_menu_item(L"Car", [](Capture& c) {
				wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
			ui.e_menu_item(L"House", [](Capture& c) {
				wprintf(L"Remove %s!\n", c.current<cEventReceiver>()->entity->get_component(cText)->text.v);
			}, Capture());
		ui.e_end_sub_menu();
	ui.e_end_popup_menu();

	ui.parents.pop();
}

MainForm::MainForm() :
	GraphicsWindow(&app, true, true, "UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable)
{
	main_window = this;

	setup_as_main_window();

	ui.init(world);

	canvas->clear_color = Vec4f(ui.style(BackgroundColor).c) / 255.f;
	canvas->set_resource(img_id, Image::create_from_file(app.graphics_device, (app.engine_path / L"art/9.png").c_str())->default_view());

	app.create_widgets();
}

MainForm::~MainForm()
{
	main_window = nullptr;
}

void MainForm::on_update()
{
	if (swapchain_image_index >= 0)
	{
		auto color = ui.style(ForegroundColor).c;
		{
			std::vector<Vec2f> points;
			path_bezier(points, Vec2f(20.f, 500.f), Vec2f(70.f, 450.f), Vec2f(120.f, 550.f), Vec2f(170.f, 500.f));
			canvas->stroke(points.size(), points.data(), color, 1.f);
		}
		{
			std::vector<Vec2f> points;
			path_bezier(points, Vec2f(20.f, 550.f), Vec2f(70.f, 500.f), Vec2f(120.f, 600.f), Vec2f(170.f, 550.f));
			canvas->stroke(points.size(), points.data(), color, 2.f);
		}
		{
			std::vector<Vec2f> points;
			path_bezier(points, Vec2f(20.f, 600.f), Vec2f(70.f, 550.f), Vec2f(120.f, 650.f), Vec2f(170.f, 600.f));
			canvas->stroke(points.size(), points.data(), color, 3.f);
		}
	}
}

int main(int argc, char** args)
{
	app.create();
	new MainForm();

	get_looper()->loop([](Capture&) {
		app.run();
	}, Capture());

	return 0;
}
