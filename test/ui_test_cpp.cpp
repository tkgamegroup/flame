#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/universe/ui/utils.h>
#include <flame/utils/app.h>

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct MyApp : App
{
	sEventDispatcher* event_dispatcher;

	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;

	void create_widgets()
	{
		ui::set_current_root(root);
		ui::push_parent(root);

		ui::e_begin_layout(LayoutVertical, 0.f, false, false);
		ui::c_aligner(SizeFitParent, SizeFitParent);
			ui::e_begin_menu_bar();
				ui::e_begin_menubar_menu(L"Style");
					ui::e_menu_item(L"Dark", [](void* c) {
						looper().add_event([](void*) {
							app.root->remove_child((Entity*)INVALID_POINTER);
							app.canvas->set_clear_color(Vec4c(0, 0, 0, 255));
							ui::style_set_to_dark();
							app.create_widgets();
						}, Mail<>());
					}, Mail<>());
					ui::e_menu_item(L"Light", [](void* c) {
						looper().add_event([](void*) {
							app.root->remove_child((Entity*)INVALID_POINTER);
							app.canvas->set_clear_color(Vec4c(200, 200, 200, 255));
							ui::style_set_to_light();
							app.create_widgets();
						}, Mail<>());
					}, Mail<>());
				ui::e_end_menubar_menu();
				ui::e_begin_menubar_menu(L"Window");
					ui::e_menu_item(L"Status", [](void* c) {
						ui::push_parent(app.root);
						ui::next_element_pos = Vec2f(100.f);
						auto w = ui::e_begin_window(L"Status");
						struct Capture
						{
							cText* txt_mouse;
							cText* txt_hovering;
							cText* txt_focusing;
							cText* txt_drag_overing;
						}capture;
						capture.txt_mouse = ui::e_text(L"Mouse: ")->get_component(cText);
						capture.txt_hovering = ui::e_text(L"Hovering: ")->get_component(cText);
						capture.txt_focusing = ui::e_text(L"Focusing: ")->get_component(cText);
						capture.txt_drag_overing = ui::e_text(L"Drag Overing: ")->get_component(cText);
						ui::e_end_window();
						ui::pop_parent();
						w->associate_resource(looper().add_event([](void* c) {
							auto& capture = *(Capture*)c;
							{
								std::wstring str = L"Mouse: ";
								str += to_wstring(app.event_dispatcher->mouse_pos);
								capture.txt_mouse->set_text(str.c_str());
							}
							{
								std::wstring str = L"Hovering: ";
								auto hovering = app.event_dispatcher->hovering;
								if (hovering)
								{
									if (hovering->entity == app.root)
										str += L"Root";
									else
										str += wfmt(L"%I64X", (ulonglong)hovering);
								}
								else
									str += L"NULL";
								capture.txt_hovering->set_text(str.c_str());
							}
							{
								auto color = ui::style_4c(ui::TextColorNormal);
								std::wstring str = L"Focusing: ";
								auto focusing = app.event_dispatcher->focusing;
								if (focusing)
								{
									if (focusing->entity == app.root)
										str += L"Root";
									else
										str += wfmt(L"%I64X", (ulonglong)focusing);
									if (focusing == app.event_dispatcher->hovering)
										color = Vec4c(0, 255, 0, 255);
									if (app.event_dispatcher->focusing->active)
										str += L" Active";
									if (app.event_dispatcher->focusing->dragging)
										str += L" Dragging";
								}
								else
									str += L"NULL";
								capture.txt_focusing->color = color;
								capture.txt_focusing->set_text(str.c_str());
							}
							{
								std::wstring str = L"Draw Overing: ";
								auto drag_overing = app.event_dispatcher->drag_overing;
								if (drag_overing)
								{
									if (drag_overing->entity == app.root)
										str += L"Root";
									else
										str += wfmt(L"%I64X", (ulonglong)drag_overing);
								}
								else
									str += L"NULL";
								capture.txt_drag_overing->set_text(str.c_str());
							}
						}, new_mail(&capture), nullptr, true), [](void* ev) {
							looper().remove_event(ev);
						});
					}, Mail<>());
				ui::e_end_menubar_menu();
			ui::e_end_menu_bar();

			ui::e_begin_layout();
			ui::c_aligner(SizeFitParent, SizeFitParent);
				ui::next_element_pos = Vec2f(16.f, 10.f);
				ui::e_begin_layout(LayoutVertical, 16.f);
					ui::e_text(L"Text Pixel");
					ui::push_font_atlas(font_atlas_lcd);
					ui::e_text(L"Text Lcd");
					ui::pop_font_atlas();
					ui::push_font_atlas(font_atlas_sdf);
					ui::e_text(L"Text Sdf");
					ui::pop_font_atlas();
					ui::next_entity = Entity::create();
					ui::e_button(L"Click Me£¡", [](void* c) {
						(*(Entity**)c)->get_component(cText)->set_text(L"Click Me! :)");
						printf("thank you for clicking me\n");
					}, new_mail_p(ui::next_entity));
					ui::e_checkbox(L"Checkbox");
					ui::e_toggle(L"Toggle");
					ui::e_image(img_id << 16, Vec2f(250.f), 4.f, 2.f, Vec4c(10, 200, 10, 255));
					ui::e_edit(100.f);
				ui::e_end_layout();

				ui::next_element_pos = Vec2f(416.f, 10.f);
				ui::e_begin_layout(LayoutVertical, 16.f);
					ui::e_begin_scroll_view1(ScrollbarVertical, Vec2f(200.f, 100.f), 4.f, 2.f)->get_component(cElement);
						ui::e_begin_list(true);
							for (auto i = 0; i < 10; i++)
								ui::e_list_item((L"item" + std::to_wstring(i)).c_str());
						ui::e_end_list();
					ui::e_end_scroll_view1();

					ui::e_begin_tree(false, 4.f, 2.f);
						ui::e_begin_tree_node(L"A");
							ui::e_tree_leaf(L"C");
							ui::e_tree_leaf(L"D");
						ui::e_end_tree_node();
						ui::e_tree_leaf(L"B");
					ui::e_end_tree();

					ui::e_begin_combobox(100.f);
						ui::e_combobox_item(L"Apple");
						ui::e_combobox_item(L"Boy");
						ui::e_combobox_item(L"Cat");
					ui::e_end_combobox();
				ui::e_end_layout();

				ui::e_begin_popup_menu();
				ui::next_entity = Entity::create();
					ui::e_menu_item(L"Refresh", [](void* c) {
						wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, new_mail_p(ui::next_entity));
					ui::next_entity = Entity::create();
					ui::e_menu_item(L"Save", [](void* c) {
						wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, new_mail_p(ui::next_entity));
					ui::next_entity = Entity::create();
					ui::e_menu_item(L"Help", [](void* c) {
						wprintf(L"%s!\n", (*(Entity**)c)->get_component(cText)->text());
					}, new_mail_p(ui::next_entity));
					ui::e_begin_sub_menu(L"add");
					ui::next_entity = Entity::create();
						ui::e_menu_item(L"Tree", [](void* c) {
							wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
						ui::next_entity = Entity::create();
						ui::e_menu_item(L"Car", [](void* c) {
							wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
						ui::next_entity = Entity::create();
						ui::e_menu_item(L"House", [](void* c) {
							wprintf(L"Add %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
					ui::e_end_sub_menu();
					ui::e_begin_sub_menu(L"Remove");
					ui::next_entity = Entity::create();
						ui::e_menu_item(L"Tree", [](void* c) {
							wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
						ui::next_entity = Entity::create();
						ui::e_menu_item(L"Car", [](void* c) {
							wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
						ui::next_entity = Entity::create();
						ui::e_menu_item(L"House", [](void* c) {
							wprintf(L"Remove %s!\n", (*(Entity**)c)->get_component(cText)->text());
						}, new_mail_p(ui::next_entity));
					ui::e_end_sub_menu();
				ui::e_end_popup_menu();

			ui::e_end_layout();

			ui::e_text(L"");
			ui::current_entity()->associate_resource(add_fps_listener([](void* c, uint fps) {
				(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
			}, new_mail_p(ui::current_entity()->get_component(cText))), [](void* ev) {
				looper().remove_event(ev);
			});
		ui::e_end_layout();

		//ui::e_begin_docker_static_container();
		//ui::e_end_docker_static_container();

		ui::next_element_pos = Vec2f(416.f, 300.f);
		ui::next_element_size = Vec2f(200.f, 200.f);
		ui::e_begin_docker_floating_container();
			ui::e_begin_docker();
				ui::e_begin_docker_page(L"ResourceExplorer");
					ui::e_text(L"flower.png  main.cpp");
				ui::e_end_docker_page();
			ui::e_end_docker();
		ui::e_end_docker_floating_container();

		ui::next_element_pos = Vec2f(640.f, 300.f);
		ui::next_element_size = Vec2f(200.f, 200.f);
		ui::e_begin_docker_floating_container();
			ui::e_begin_docker_layout(LayoutHorizontal);
				ui::e_begin_docker();
					ui::e_begin_docker_page(L"TextEditor");
						ui::e_text(L"printf(\"Hello World!\\n\");");
					ui::e_end_docker_page();
				ui::e_end_docker();
				ui::e_begin_docker_layout(LayoutVertical);
					ui::e_begin_docker();
						ui::e_begin_docker_page(L"Hierarchy");
							ui::e_text(L"Node A\n--Node B");
						ui::e_end_docker_page();
					ui::e_end_docker();
					ui::e_begin_docker();
						ui::e_begin_docker_page(L"Inspector");
							ui::e_text(L"Name: James Bond\nID: 007");
						ui::e_end_docker_page();
					ui::e_end_docker();
				ui::e_end_docker_layout();
			ui::e_end_docker_layout();
		ui::e_end_docker_floating_container();

		ui::pop_parent();
	}
}app;

int main(int argc, char** args)
{
	app.create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);

	auto w = app.u->world(0);

	app.event_dispatcher = w->get_system(sEventDispatcher);
	{
		wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc",
			L"../art/font_awesome.ttf"
		};
		app.font_atlas_lcd = FontAtlas::create(app.d, FontDrawLcd, 1, fonts);
		app.canvas->add_font(app.font_atlas_lcd);
		app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, 1, fonts);
		app.canvas->add_font(app.font_atlas_sdf);
	}
	app.canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../art/ui/imgs/9.png")));

	app.root = w->root();
	ui::set_current_entity(app.root);
	ui::c_layout();
	ui::push_font_atlas(app.font_atlas_pixel);
	app.create_widgets();

	looper().loop([](void*) {
		app.run();
	}, Mail<>());

	return 0;
}
