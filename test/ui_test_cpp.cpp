#include <flame/serialize.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/universe/world.h>
#include <flame/universe/ui/utils.h>

#include "../renderpath/canvas/canvas.h"

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct App
{
	Window* w;
	Device* d;
	SwapchainResizable* scr;
	Fence* fence;
	Array<Commandbuffer*> cbs;
	Semaphore* render_finished;

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;

	Universe* u;
	Entity* root;
	cElement* c_element_root;
	cEventReceiver* c_event_receiver_root;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_events();

		c_element_root->set_size(Vec2f(w->size));
		u->update();

		if (sc)
		{
			d->gq->submit(1, &cbs.v[sc->image_index()], sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->image_count());
	for (auto i = 0; i < app.cbs.s; i++)
		app.cbs.v[i] = Commandbuffer::create(app.d->gcp);
	app.render_finished = Semaphore::create(app.d);

	app.u = Universe::create();
	app.u->add_object(app.w);

	auto w = World::create(app.u);
	w->add_system(sLayoutManagement::create());
	w->add_system(sEventDispatcher::create());
	auto s_2d_renderer = s2DRenderer::create(L"../renderpath/canvas/bp", app.scr, FLAME_CHASH("SwapchainResizable"), &app.cbs);
	w->add_system(s_2d_renderer);
	{
		auto canvas = s_2d_renderer->canvas;
		wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc",
			L"../art/font_awesome.ttf"
		};
		app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, 2, fonts);
		canvas->add_font(app.font_atlas_pixel);
		app.font_atlas_lcd = FontAtlas::create(app.d, FontDrawLcd, 1, fonts);
		canvas->add_font(app.font_atlas_lcd);
		app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, 1, fonts);
		canvas->add_font(app.font_atlas_sdf);

		canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../art/ui/imgs/9.png")));
	}

	auto root = w->root();
	app.root = root;

	ui::set_current_entity(root);
	app.c_element_root = ui::c_element();
	app.c_event_receiver_root = ui::c_event_receiver();
	ui::c_layout();

	ui::push_font_atlas(app.font_atlas_pixel);
	ui::push_parent(root);

	ui::e_text(L"");
	ui::c_aligner(AlignxLeft, AlignyBottom);
	add_fps_listener([](void* c, uint fps) {
		(*(cText**)c)->set_text(std::to_wstring(fps).c_str());
	}, new_mail_p(ui::current_entity()->get_component(cText)));

	ui::e_begin_layout(16.f, 28.f, LayoutVertical, 16.f);

	ui::e_text(L"Text Pixel");
	ui::push_font_atlas(app.font_atlas_lcd);
	ui::e_text(L"Text Lcd");
	ui::pop_font_atlas();
	ui::push_font_atlas(app.font_atlas_sdf);
	ui::e_text(L"Text Sdf");
	ui::pop_font_atlas();
	ui::e_button(L"Click Me£¡", [](void* c, Entity* e) {
		e->get_component(cText)->set_text(L"Click Me! :)");
		printf("thank you for clicking me\n");
	}, Mail<>());
	ui::e_checkbox(L"Checkbox");
	ui::e_toggle(L"Toggle");
	ui::e_image(img_id << 16, Vec2f(250.f), 4.f, 2.f, Vec4c(10, 200, 10, 255));
	ui::e_edit(100.f);

	ui::e_end_layout();

	ui::e_begin_layout(416.f, 28.f, LayoutVertical, 16.f);
	ui::e_end_layout();

	ui::pop_parent();
	ui::pop_font_atlas();

	/*

	{
		auto e_list = create_standard_list(true);

		for (auto i = 0; i < 10; i++)
		{
			auto e_item = create_standard_listitem(app.font_atlas_pixel, 1.f, (L"item" + std::to_wstring(i)).c_str());
			e_list->add_child(e_item);
		}

		auto e_container = wrap_standard_scrollbar(e_list, ScrollbarVertical, false, 1.f);
		e_layout_right->add_child(e_container);
		{
			auto c_element = e_container->get_component(cElement);
			c_element->size_ = Vec2f(200.f, 100.f);
			c_element->inner_padding_ = Vec4f(4.f);
			c_element->frame_thickness_ = 2.f;
		}
	}

	auto e_popup_menu = create_standard_menu();
	{
		for (auto i = 0; i < 3; i++)
		{
			char* names[] = {
				"Refresh",
				"Save",
				"Help"
			};
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
			e_popup_menu->add_child(e_item);
			e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					printf("%s!\n", *(char**)c);
					ui::remove_top_layer(app.root);
				}
			}, new_mail_p(names[i]));
		}

		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 3; i++)
			{
				char* names[] = {
					"Tree",
					"Car",
					"House"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
				e_menu->add_child(e_item);
				e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Add %s!\n", *(char**)c);
						ui::remove_top_layer(app.root);
					}
				}, new_mail_p(names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add", root, e_menu, true, SideE, false, true, false, Icon_CARET_RIGHT);
			e_popup_menu->add_child(e_menu_btn);
		}

		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 3; i++)
			{
				char* names[] = {
					"Tree",
					"Car",
					"House"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
				e_menu->add_child(e_item);
				e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Remove %s!\n", *(char**)c);
						ui::remove_top_layer(app.root);
					}
				}, new_mail_p(names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Remove", root, e_menu, true, SideE, false, true, false, Icon_CARET_RIGHT);
			e_popup_menu->add_child(e_menu_btn);
		}
	}

	{
		struct Capture
		{
			Entity* menu;
			Entity* root;
		}capture;
		capture.menu = e_popup_menu;
		capture.root = root;
		app.c_event_receiver_root->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_down(action, key, true) && key == Mouse_Right)
			{
				auto& capture = *(Capture*)c;
				popup_menu(capture.menu, capture.root, (Vec2f)pos);
			}
		}, new_mail(&capture));
	}
	
	auto e_menubar = create_standard_menubar();
	root->add_child(e_menubar);

	{
		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 2; i++)
			{
				char* names[] = {
					"New",
					"Open"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
				e_menu->add_child(e_item);
				e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						ui::remove_top_layer(app.root);
					}
				}, new_mail_p(names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"File", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 6; i++)
			{
				char* names[] = {
					"Undo",
					"Redo",
					"Cut",
					"Copy",
					"Paste",
					"Delete"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
				e_menu->add_child(e_item);
				e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						ui::remove_top_layer(app.root);
					}
				}, new_mail_p(names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 2; i++)
			{
				char* names[] = {
					"Monitor",
					"Console"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]).c_str());
				e_menu->add_child(e_item);
				e_item->get_component(cEventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						ui::remove_top_layer(app.root);
					}
				}, new_mail_p(names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Tool", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
	}

	{
		wchar_t* items[] = {
			L"Apple", L"Boy", L"Cat"
		};
		auto e_combobox = create_standard_combobox(100.f, app.font_atlas_pixel, 1.f, root, 3, items);
		e_layout_right->add_child(e_combobox);
	}

	auto e_tree = create_standard_tree(false);
	e_layout_right->add_child(e_tree);
	{
		auto c_element = e_tree->get_component(cElement);
		c_element->inner_padding_ = Vec4f(4.f);
		c_element->frame_thickness_ = 2.f;
	}
	{
		auto e_tree_node = create_standard_tree_node(app.font_atlas_pixel, L"A");
		e_tree->add_child(e_tree_node);
		auto e_sub_tree = e_tree_node->child(1);
		{
			auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, L"C");
			e_sub_tree->add_child(e_tree_leaf);
		}
		{
			auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, L"D");
			e_sub_tree->add_child(e_tree_leaf);
		}
	}
	{
		auto e_tree_leaf = create_standard_tree_leaf(app.font_atlas_pixel, L"B");
		e_tree->add_child(e_tree_leaf);
	}

	{
		auto e_container = get_docker_container_model()->copy();
		root->add_child(e_container);
		{
			auto c_element = e_container->get_component(cElement);
			c_element->pos_.x() = 414.f;
			c_element->pos_.y() = 297.f;
			c_element->size_.x() = 221.f;
			c_element->size_.y() = 214.f;
		}

		auto e_docker = get_docker_model()->copy();
		e_container->add_child(e_docker, 0);
		auto e_tabbar = e_docker->child(0);
		auto e_pages = e_docker->child(1);

		for (auto i = 0; i < 1; i++)
		{
			wchar_t* names[] = {
				L"Hierarchy",
			};

			e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->auto_width_ = false;
				c_text->auto_height_ = false;
				c_text->set_text(names[i]);
				e_page->add_component(c_text);
			}
		}
	}

	{
		auto e_container = get_docker_container_model()->copy();
		root->add_child(e_container);
		{
			auto c_element = e_container->get_component(cElement);
			c_element->pos_ = Vec2f(667.f, 302.f);
			c_element->size_ = Vec2f(403.f, 215.f);
		}

		auto e_docker_layout = get_docker_layout_model()->copy();
		e_container->add_child(e_docker_layout, 0);

		{
			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 0);
				{
					auto c_aligner = e_docker->get_component(cAligner);
					c_aligner->x_align_ = AlignxFree;
					c_aligner->y_align_ = AlignyFree;
					c_aligner->using_padding_ = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					wchar_t* names[] = {
						L"Inspector",
						L"ResourceExplorer"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_width_ = false;
						c_text->auto_height_ = false;
						c_text->set_text(names[i]);
						e_page->add_component(c_text);
					}
				}
			}

			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 2);
				{
					auto c_aligner = e_docker->get_component(cAligner);
					c_aligner->x_align_ = AlignxFree;
					c_aligner->y_align_ = AlignyFree;
					c_aligner->using_padding_ = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					wchar_t* names[] = {
						L"TextEditor",
						L"ShaderEditor"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_width_ = false;
						c_text->auto_height_ = false;
						c_text->set_text(names[i]);
						e_page->add_component(c_text);
					}
				}
			}
		}
	}
	*/

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
