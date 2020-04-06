#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/utils/ui.h>
#include <flame/utils/app.h>
#include <flame/utils/fps.h>

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct MyApp : App
{
	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;

	void create_widgets()
	{
		utils::set_current_root(root);
		utils::push_parent(root);

			utils::e_begin_layout(LayoutVertical, 0.f, false, false);
			utils::c_aligner(SizeFitParent, SizeFitParent);
				utils::e_begin_menu_bar();
					utils::e_begin_menubar_menu(L"Style");
						utils::e_menu_item(L"Dark", [](void* c) {
							looper().add_event([](void*, bool*) {
								app.root->remove_children(0, -1);
								utils::style_set_to_dark();
								app.canvas->set_clear_color(utils::style_4c(utils::BackgroundColor));
								app.create_widgets();
							}, Mail());
						}, Mail());
						utils::e_menu_item(L"Light", [](void* c) {
							looper().add_event([](void*, bool*) {
								app.root->remove_children(0, -1);
								utils::style_set_to_light();
								app.canvas->set_clear_color(utils::style_4c(utils::BackgroundColor));
								app.create_widgets();
							}, Mail());
						}, Mail());
					utils::e_end_menubar_menu();
					utils::e_begin_menubar_menu(L"Window");
						utils::e_menu_item(L"Reflector", [](void* c) {
							utils::next_element_pos = Vec2f(100.f);
							utils::e_reflector_window(app.s_event_dispatcher);
						}, Mail());
					utils::e_end_menubar_menu();
				utils::e_end_menu_bar();

				utils::e_begin_layout();
				utils::c_aligner(SizeFitParent, SizeFitParent);
					utils::next_element_pos = Vec2f(16.f, 10.f);
					utils::e_begin_layout(LayoutVertical, 16.f);
						utils::e_text(L"Text Pixel");
						utils::push_font_atlas(font_atlas_lcd);
						utils::e_text(L"Text Lcd");
						utils::pop_font_atlas();
						utils::push_font_atlas(font_atlas_sdf);
						utils::e_text(L"Text Sdf");
						utils::pop_font_atlas();
						utils::next_entity = Entity::create();
						utils::e_button(L"Click Me!", [](void* c) {
							(*(Entity**)c)->get_component(cText)->set_text(L"Click Me! :)");
							printf("thank you for clicking me\n");
						}, Mail::from_p(utils::next_entity));
						utils::e_checkbox(L"Checkbox");
						utils::next_element_size = 250.f;
						{
							auto c_element = utils::e_image(img_id << 16, 4.f)->get_component(cElement);
							c_element->frame_thickness_ = 2.f;
							c_element->frame_color_ = utils::style_4c(utils::ForegroundColor);
						}
						utils::e_edit(100.f);
					utils::e_end_layout();

					utils::next_element_pos = Vec2f(416.f, 10.f);
					utils::e_begin_layout(LayoutVertical, 16.f);
						{
							auto c_element = utils::e_begin_scroll_view1(ScrollbarVertical, Vec2f(200.f, 100.f), 4.f)->get_component(cElement);
							c_element->frame_thickness_ = 2.f;
							c_element->frame_color_ = utils::style_4c(utils::ForegroundColor);
							utils::e_begin_list(true);
							for (auto i = 0; i < 10; i++)
								utils::e_list_item((L"item" + std::to_wstring(i)).c_str());
							utils::e_end_list();
							utils::e_end_scroll_view1();
						}

						{
							auto c_element = utils::e_begin_tree(false, 4.f)->get_component(cElement);
							c_element->frame_thickness_ = 2.f;
							c_element->frame_color_ = utils::style_4c(utils::ForegroundColor);
								utils::e_begin_tree_node(L"A");
									utils::e_tree_leaf(L"C");
									utils::e_tree_leaf(L"D");
								utils::e_end_tree_node();
								utils::e_tree_leaf(L"B");
							utils::e_end_tree();
						}

						utils::e_begin_combobox();
							utils::e_combobox_item(L"Apple");
							utils::e_combobox_item(L"Boy");
							utils::e_combobox_item(L"Cat");
						utils::e_end_combobox();
					utils::e_end_layout();

				utils::e_end_layout();

				{
					auto e = utils::e_text(L"");
					e->on_destroyed_listeners.add([](void* c) {
						looper().remove_event(*(void**)c);
						return true;
					}, Mail::from_p(add_fps_listener([](void* c, uint fps) {
						(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
					}, Mail::from_p(e->get_component(cText)))));
				}
			utils::e_end_layout();

			//utils::e_begin_docker_static_container();
			//utils::e_end_docker_static_container();

			utils::next_element_pos = Vec2f(416.f, 300.f);
			utils::next_element_size = Vec2f(200.f, 200.f);
			utils::e_begin_docker_floating_container();
				utils::e_begin_docker();
					utils::e_begin_docker_page(L"ResourceExplorer").second->get_component(cElement)->color_ = utils::style_4c(utils::FrameColorNormal);
						utils::e_text(L"flower.png  main.cpp");
					utils::e_end_docker_page();
				utils::e_end_docker();
			utils::e_end_docker_floating_container();

			utils::next_element_pos = Vec2f(640.f, 300.f);
			utils::next_element_size = Vec2f(200.f, 200.f);
			utils::e_begin_docker_floating_container();
				utils::e_begin_docker_layout(LayoutHorizontal);
					utils::e_begin_docker();
						utils::e_begin_docker_page(L"TextEditor").second->get_component(cElement)->color_ = utils::style_4c(utils::FrameColorNormal);
							utils::e_text(L"printf(\"Hello World!\\n\");");
						utils::e_end_docker_page();
					utils::e_end_docker();
					utils::e_begin_docker_layout(LayoutVertical);
						utils::e_begin_docker();
							utils::e_begin_docker_page(L"Hierarchy").second->get_component(cElement)->color_ = utils::style_4c(utils::FrameColorNormal);
								utils::e_text(L"Node A\n--Node B");
							utils::e_end_docker_page();
						utils::e_end_docker();
						utils::e_begin_docker();
							utils::e_begin_docker_page(L"Inspector").second->get_component(cElement)->color_ = utils::style_4c(utils::FrameColorNormal);
								utils::e_text(L"Name: James Bond\nID: 007");
							utils::e_end_docker_page();
						utils::e_end_docker();
					utils::e_end_docker_layout();
				utils::e_end_docker_layout();
			utils::e_end_docker_floating_container();

			utils::e_begin_popup_menu();
				utils::next_entity = Entity::create();
				utils::e_menu_item(L"Refresh", [](void* c) {
					wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
				}, Mail::from_p(utils::next_entity));
				utils::next_entity = Entity::create();
				utils::e_menu_item(L"Save", [](void* c) {
					wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
				}, Mail::from_p(utils::next_entity));
				utils::next_entity = Entity::create();
				utils::e_menu_item(L"Help", [](void* c) {
					wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
				}, Mail::from_p(utils::next_entity));
				utils::e_begin_sub_menu(L"Add");
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"Tree", [](void* c) {
						wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"Car", [](void* c) {
						wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"House", [](void* c) {
						wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
				utils::e_end_sub_menu();
				utils::e_begin_sub_menu(L"Remove");
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"Tree", [](void* c) {
						wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"Car", [](void* c) {
						wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
					utils::next_entity = Entity::create();
					utils::e_menu_item(L"House", [](void* c) {
						wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, Mail::from_p(utils::next_entity));
				utils::e_end_sub_menu();
			utils::e_end_popup_menu();

		utils::pop_parent();
	}
}app;

int main(int argc, char** args)
{
	std::filesystem::path engine_path = getenv("FLAME_PATH");

	app.create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable, true, engine_path);

	{
		wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc",
			L"../art/font_awesome.ttf"
		};
		app.font_atlas_lcd = FontAtlas::create(app.graphics_device, FontDrawLcd, 1, fonts);
		app.canvas->add_font(app.font_atlas_lcd);
		app.font_atlas_sdf = FontAtlas::create(app.graphics_device, FontDrawSdf, 1, fonts);
		app.canvas->add_font(app.font_atlas_sdf);
	}
	app.canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.graphics_device, (engine_path / L"art/9.png").c_str())));

	utils::set_current_entity(app.root);
	utils::c_event_receiver();
	utils::c_layout();
	utils::push_font_atlas(app.font_atlas_pixel);
	app.create_widgets();

	looper().loop([](void*) {
		app.run();
	}, Mail());

	return 0;
}
