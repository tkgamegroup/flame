#include <flame/foundation/serialize.h>
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
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/ui_renderer.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/checkbox.h>
#include <flame/universe/components/toggle.h>
#include <flame/universe/components/image.h>
#include <flame/universe/components/edit.h>
#include <flame/universe/components/list.h>
#include <flame/universe/components/scrollbar.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/components/combobox.h>
#include <flame/universe/components/tree.h>
#include <flame/universe/components/splitter.h>
#include <flame/universe/components/window.h>

#include "../renderpath/canvas_make_cmd/canvas.h"

using namespace flame;
using namespace graphics;

const auto img_id = 9;

struct App
{
	Window* w;
	Device* d;
	Semaphore* render_finished;
	SwapchainResizable* scr;
	Fence* fence;
	std::vector<Commandbuffer*> cbs;
	BP* bp;
	Canvas* canvas;

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;

	Universe* u;
	Entity* root;
	cElement* c_element_root;
	cEventReceiver* c_event_receiver_root;
	cText* c_text_fps;

	void run()
	{
		auto sc = scr->sc();

		if (sc)
			sc->acquire_image();

		fence->wait();
		looper().process_delay_events();

		if (sc)
		{
			c_element_root->size = w->size;
			c_text_fps->set_text(std::to_wstring(looper().fps));
			u->update();
		}
		bp->update();

		if (sc)
		{
			d->gq->submit({ (Commandbuffer*)cbs[sc->image_index()] }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}

		scr->signal = false;
	}
}app;

int main(int argc, char** args)
{
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	app.cbs.resize(app.scr->sc()->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	app.bp = BP::create_from_file(L"../renderpath/canvas_make_cmd/bp", true);
	app.bp->graphics_device = app.d;
	auto n_scr = app.bp->add_node(cH("graphics::SwapchainResizable"), "scr");
	n_scr->find_input("in")->set_data_p(app.scr);
	app.bp->find_input("*.rt_dst.type")->set_data_i(TargetImages);
	app.bp->find_input("*.rt_dst.v")->link_to(n_scr->find_output("images"));
	app.bp->find_input("*.make_cmd.cbs")->set_data_p(&app.cbs);
	app.bp->find_input("*.make_cmd.image_idx")->link_to(n_scr->find_output("image_idx"));
	app.bp->update();
	app.canvas = (Canvas*)app.bp->find_output("*.make_cmd.canvas")->data_p();

	auto font_msyh14 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
	auto font_awesome14 = Font::create(L"../asset/font_awesome.ttf", 14);
	auto font_msyh32 = Font::create(L"c:/windows/fonts/msyh.ttc", 32);
	app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, { font_msyh14, font_awesome14 });
	app.font_atlas_lcd = FontAtlas::create(app.d, FontDrawLcd, { font_msyh14 });
	app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, { font_msyh32 });
	app.font_atlas_pixel->index = app.canvas->set_image(-1, app.font_atlas_pixel->imageview());
	app.font_atlas_lcd->index = app.canvas->set_image(-1, app.font_atlas_lcd->imageview());
	app.font_atlas_sdf->index = app.canvas->set_image(-1, app.font_atlas_sdf->imageview());
	
	app.canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../asset/ui/imgs/9.png")));

	app.u = Universe::create();

	auto w = World::create();
	w->add_system(sLayoutManagement::create());
	w->add_system(sUIRenderer::create(app.canvas));
	w->add_system(sEventDispatcher::create(app.w));
	app.u->add_world(w);

	auto root = w->root();
	app.root = root;
	{
		app.c_element_root = cElement::create();
		root->add_component(app.c_element_root);

		app.c_event_receiver_root = cEventReceiver::create();
		root->add_component(app.c_event_receiver_root);

		root->add_component(cLayout::create(LayoutFree));
	}

	auto e_fps = Entity::create();
	root->add_child(e_fps);
	{
		e_fps->add_component(cElement::create());

		app.c_text_fps = cText::create(app.font_atlas_pixel);
		e_fps->add_component(app.c_text_fps);

		auto c_aligner = cAligner::create();
		c_aligner->x_align = AlignxLeft;
		c_aligner->y_align = AlignyBottom;
		e_fps->add_component(c_aligner);
	}

	auto e_layout_left = Entity::create();
	root->add_child(e_layout_left);
	{
		auto c_element = cElement::create();
		c_element->pos.x() = 16.f;
		c_element->pos.y() = 28.f;
		e_layout_left->add_component(c_element);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 16.f;
		e_layout_left->add_component(c_layout);
	}

	auto e_text_pixel = Entity::create();
	e_layout_left->add_child(e_text_pixel);
	{
		e_text_pixel->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_pixel);
		c_text->set_text(L"Text Pixel");
		e_text_pixel->add_component(c_text);
	}

	auto e_text_lcd = Entity::create();
	e_layout_left->add_child(e_text_lcd);
	{
		e_text_lcd->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_lcd);
		c_text->set_text(L"Text Lcd");
		e_text_lcd->add_component(c_text);
	}

	auto e_text_sdf = Entity::create();
	e_layout_left->add_child(e_text_sdf);
	{
		e_text_sdf->add_component(cElement::create());

		auto c_text = cText::create(app.font_atlas_sdf);
		c_text->set_text(L"Text Sdf");
		c_text->sdf_scale = 14.f / 32.f;
		e_text_sdf->add_component(c_text);
	}

	auto e_button = create_standard_button(app.font_atlas_pixel, 1.f, L"Click Me!");
	e_layout_left->add_child(e_button);
	{
		e_button->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
			if (is_mouse_clicked(action, key))
			{
				(*(cText**)c)->set_text(L"Click Me! :)");
				printf("thank you for clicking me\n");
			}
		}, new_mail_p(e_button->get_component(Text)));
	}

	auto e_checkbox = wrap_standard_text(create_standard_checkbox(), false, app.font_atlas_pixel, 1.f, L"Checkbox");
	e_layout_left->add_child(e_checkbox);

	auto e_toggle = Entity::create();
	e_layout_left->add_child(e_toggle);
	{
		auto c_element = cElement::create();
		auto r = app.font_atlas_pixel->pixel_height * 0.5f;
		c_element->roundness = r;
		c_element->inner_padding = Vec4f(r, 2.f, r, 2.f);
		e_toggle->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_pixel);
		c_text->set_text(L"Toggle");
		e_toggle->add_component(c_text);

		e_toggle->add_component(cEventReceiver::create());

		e_toggle->add_component(cStyleColor::create());

		e_toggle->add_component(cToggle::create());
	}

	auto e_image = Entity::create();
	e_layout_left->add_child(e_image);
	{
		auto c_element = cElement::create();
		c_element->size = 258.f;
		c_element->inner_padding = Vec4f(4.f);
		c_element->frame_color = Vec4c(10, 200, 10, 255);
		c_element->frame_thickness = 2.f;
		e_image->add_component(c_element);

		auto c_image = cImage::create();
		c_image->id = img_id;
		e_image->add_component(c_image);
	}

	auto e_edit = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
	e_layout_left->add_child(e_edit);

	auto e_layout_right = Entity::create();
	root->add_child(e_layout_right);
	{
		auto c_element = cElement::create();
		c_element->pos.x() = 416.f;
		c_element->pos.y() = 28.f;
		e_layout_right->add_component(c_element);

		auto c_layout = cLayout::create(LayoutVertical);
		c_layout->item_padding = 16.f;
		e_layout_right->add_component(c_layout);
	}

	{
		auto e_list = create_standard_list(true);

		for (auto i = 0; i < 10; i++)
		{
			auto e_item = create_standard_listitem(app.font_atlas_pixel, 1.f, L"item" + std::to_wstring(i));
			e_list->add_child(e_item);
		}

		auto e_container = wrap_standard_scrollbar(e_list, ScrollbarVertical, false, 1.f);
		e_layout_right->add_child(e_container);
		{
			auto c_element = e_container->get_component(Element);
			c_element->size.x() = 200.f;
			c_element->size.y() = 100.f;
			c_element->inner_padding = Vec4f(4.f);
			c_element->frame_thickness = 2.f;
		}
	}

	auto e_popup_menu = create_standard_menu();
	{
		for (auto i = 0; i < 3; i++)
		{
			static const char* names[] = {
				"Refresh",
				"Save",
				"Help"
			};
			auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
			e_popup_menu->add_child(e_item);
			e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
				if (is_mouse_down(action, key, true) && key == Mouse_Left)
				{
					printf("%s!\n", *(char**)c);
					destroy_topmost(app.root);
				}
			}, new_mail_p((char*)names[i]));
		}

		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 3; i++)
			{
				static const char* names[] = {
					"Tree",
					"Car",
					"House"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
				e_menu->add_child(e_item);
				e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Add %s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add", root, e_menu, true, SideE, false, true, false, Icon_CARET_RIGHT);
			e_popup_menu->add_child(e_menu_btn);
		}

		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 3; i++)
			{
				static const char* names[] = {
					"Tree",
					"Car",
					"House"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
				e_menu->add_child(e_item);
				e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Remove %s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
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
				static const char* names[] = {
					"New",
					"Open"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
				e_menu->add_child(e_item);
				e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"File", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 6; i++)
			{
				static const char* names[] = {
					"Undo",
					"Redo",
					"Cut",
					"Copy",
					"Paste",
					"Delete"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
				e_menu->add_child(e_item);
				e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
		{
			auto e_menu = create_standard_menu();
			for (auto i = 0; i < 2; i++)
			{
				static const char* names[] = {
					"Monitor",
					"Console"
				};
				auto e_item = create_standard_menu_item(app.font_atlas_pixel, 1.f, s2w(names[i]));
				e_menu->add_child(e_item);
				e_item->get_component(EventReceiver)->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Tool", root, e_menu, true, SideS, true, false, true, nullptr);
			e_menubar->add_child(e_menu_btn);
		}
	}

	auto e_combobox = create_standard_combobox(100.f, app.font_atlas_pixel, 1.f, root, { L"Apple", L"Boy", L"Cat" });
	e_layout_right->add_child(e_combobox);

	auto e_tree = create_standard_tree(false);
	e_layout_right->add_child(e_tree);
	{
		auto c_element = e_tree->get_component(Element);
		c_element->inner_padding = Vec4f(4.f);
		c_element->frame_thickness = 2.f;
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
			auto c_element = e_container->get_component(Element);
			c_element->pos.x() = 414.f;
			c_element->pos.y() = 297.f;
			c_element->size.x() = 221.f;
			c_element->size.y() = 214.f;
		}

		auto e_docker = get_docker_model()->copy();
		e_container->add_child(e_docker, 0);
		auto e_tabbar = e_docker->child(0);
		auto e_pages = e_docker->child(1);

		for (auto i = 0; i < 1; i++)
		{
			static const wchar_t* names[] = {
				L"Hierarchy",
			};

			e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->auto_width = false;
				c_text->auto_height = false;
				c_text->set_text(names[i]);
				e_page->add_component(c_text);
			}
		}
	}

	{
		auto e_container = get_docker_container_model()->copy();
		root->add_child(e_container);
		{
			auto c_element = e_container->get_component(Element);
			c_element->pos.x() = 667.f;
			c_element->pos.y() = 302.f;
			c_element->size.x() = 403.f;
			c_element->size.y() = 215.f;
		}

		auto e_docker_layout = get_docker_layout_model()->copy();
		e_container->add_child(e_docker_layout, 0);

		{
			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 0);
				{
					auto c_aligner = e_docker->get_component(Aligner);
					c_aligner->x_align = AlignxFree;
					c_aligner->y_align = AlignyFree;
					c_aligner->using_padding = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					static const wchar_t* names[] = {
						L"Inspector",
						L"ResourceExplorer"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_width = false;
						c_text->auto_height = false;
						c_text->set_text(names[i]);
						e_page->add_component(c_text);
					}
				}
			}

			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 2);
				{
					auto c_aligner = e_docker->get_component(Aligner);
					c_aligner->x_align = AlignxFree;
					c_aligner->y_align = AlignyFree;
					c_aligner->using_padding = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					static const wchar_t* names[] = {
						L"TextEditor",
						L"ShaderEditor"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_width = false;
						c_text->auto_height = false;
						c_text->set_text(names[i]);
						e_page->add_component(c_text);
					}
				}
			}
		}
	}

	looper().loop([](void* c) {
		auto app = (*(App**)c);
		app->run();
	}, new_mail_p(&app));

	return 0;
}
