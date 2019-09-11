#include <flame/foundation/serialize.h>
#include <flame/graphics/device.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/shader.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_dispatcher.h>
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

	FontAtlas* font_atlas_pixel;
	FontAtlas* font_atlas_lcd;
	FontAtlas* font_atlas_sdf;
	Canvas* canvas;
	int rt_frame;

	Entity* root;
	cElement* c_element_root;
	cEventReceiver* c_event_receiver_root;
	cText* c_text_fps;

	void run()
	{
		auto sc = scr->sc();
		auto sc_frame = scr->sc_frame();

		if (sc_frame > rt_frame)
		{
			canvas->set_render_target(TargetImages, sc ? &sc->images() : nullptr);
			rt_frame = sc_frame;
		}

		if (sc)
		{
			sc->acquire_image();
			fence->wait();

			c_element_root->width = w->size.x();
			c_element_root->height = w->size.y();
			c_text_fps->set_text(std::to_wstring(looper().fps));
			root->update();

			auto img_idx = sc->image_index();
			auto cb = cbs[img_idx];
			canvas->record(cb, img_idx);

			d->gq->submit({ cb }, sc->image_avalible(), render_finished, fence);
			d->gq->present(sc, render_finished);
		}
	}
}app;

int main(int argc, char** args)
{
	app.w = Window::create("UI Test", Vec2u(1280, 720), WindowFrame | WindowResizable);
	app.d = Device::create(true);
	app.render_finished = Semaphore::create(app.d);
	app.scr = SwapchainResizable::create(app.d, app.w);
	app.fence = Fence::create(app.d);
	auto sc = app.scr->sc();
	app.canvas = Canvas::create(app.d, TargetImages, &sc->images());
	app.cbs.resize(sc->images().size());
	for (auto i = 0; i < app.cbs.size(); i++)
		app.cbs[i] = Commandbuffer::create(app.d->gcp);

	auto font_msyh14 = Font::create(L"c:/windows/fonts/msyh.ttc", 14);
	auto font_awesome14 = Font::create(L"../asset/font_awesome.ttf", 14);
	auto font_msyh32 = Font::create(L"c:/windows/fonts/msyh.ttc", 32);
	app.font_atlas_pixel = FontAtlas::create(app.d, FontDrawPixel, { font_msyh14, font_awesome14 });
	app.font_atlas_lcd = FontAtlas::create(app.d, FontDrawLcd, { font_msyh14 });
	app.font_atlas_sdf = FontAtlas::create(app.d, FontDrawSdf, { font_msyh32 });
	app.font_atlas_pixel->index = 1;
	app.font_atlas_lcd->index = 2;
	app.font_atlas_sdf->index = 3;
	app.canvas->set_image(app.font_atlas_pixel->index, Imageview::create(app.font_atlas_pixel->image(), Imageview2D, 0, 1, 0, 1, SwizzleOne, SwizzleOne, SwizzleOne, SwizzleR));
	app.canvas->set_image(app.font_atlas_lcd->index, Imageview::create(app.font_atlas_lcd->image()));
	app.canvas->set_image(app.font_atlas_sdf->index, Imageview::create(app.font_atlas_sdf->image()));

	app.canvas->set_image(img_id, Imageview::create(Image::create_from_file(app.d, L"../asset/ui/imgs/9.png")));

	app.root = Entity::create();
	{
		app.c_element_root = cElement::create(app.canvas);
		app.root->add_component(app.c_element_root);

		app.root->add_component(cEventDispatcher::create(app.w));

		app.c_event_receiver_root = cEventReceiver::create();
		app.root->add_component(app.c_event_receiver_root);

		app.root->add_component(cLayout::create());
	}

	auto e_fps = Entity::create();
	app.root->add_child(e_fps);
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
	app.root->add_child(e_layout_left);
	{
		auto c_element = cElement::create();
		c_element->x = 16.f;
		c_element->y = 28.f;
		e_layout_left->add_component(c_element);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
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
		auto c_event_receiver = (cEventReceiver*)e_button->find_component(cH("EventReceiver"));
		c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			if (is_mouse_clicked(action, key))
			{
				auto thiz = *(App**)c;
				printf("thank you for clicking me\n");
			}
		}, new_mail_p(&app));
	}

	auto e_checkbox = create_standard_checkbox(app.font_atlas_pixel, 1.f, L"Checkbox", true);
	e_layout_left->add_child(e_checkbox);

	auto e_toggle = Entity::create();
	e_layout_left->add_child(e_toggle);
	{
		auto c_element = cElement::create();
		c_element->background_round_flags = SideNW | SideNE | SideSW | SideSE;
		c_element->background_round_radius = app.font_atlas_pixel->pixel_height * 0.5f;
		c_element->inner_padding = Vec4f(c_element->background_round_radius, 2.f, c_element->background_round_radius, 2.f);
		e_toggle->add_component(c_element);

		auto c_text = cText::create(app.font_atlas_pixel);
		c_text->set_text(L"Toggle");
		e_toggle->add_component(c_text);

		e_toggle->add_component(cEventReceiver::create());

		e_toggle->add_component(cStyleBackgroundColor::create(Vec4c(0), Vec4c(0), Vec4c(0)));

		e_toggle->add_component(cToggle::create());
	}

	auto e_image = Entity::create();
	e_layout_left->add_child(e_image);
	{
		auto c_element = cElement::create();
		c_element->width = 258.f;
		c_element->height = 258.f;
		c_element->inner_padding = Vec4f(4.f);
		c_element->background_frame_color = Vec4c(10, 200, 10, 255);
		c_element->background_frame_thickness = 2.f;
		e_image->add_component(c_element);

		auto c_image = cImage::create();
		c_image->id = img_id;
		e_image->add_component(c_image);
	}

	auto e_edit = create_standard_edit(100.f, app.font_atlas_pixel, 1.f);
	e_layout_left->add_child(e_edit);

	auto e_layout_right = Entity::create();
	app.root->add_child(e_layout_right);
	{
		auto c_element = cElement::create();
		c_element->x = 416.f;
		c_element->y = 28.f;
		e_layout_right->add_component(c_element);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 16.f;
		e_layout_right->add_component(c_layout);
	}

	{
		auto e_list = Entity::create();
		{
			e_list->add_component(cElement::create());

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			c_aligner->height_policy = SizeFitParent;
			e_list->add_component(c_aligner);

			auto c_layout = cLayout::create();
			c_layout->type = LayoutVertical;
			c_layout->item_padding = 4.f;
			c_layout->width_fit_children = false;
			c_layout->height_fit_children = false;
			e_list->add_component(c_layout);

			e_list->add_component(cList::create());
		}

		for (auto i = 0; i < 10; i++)
		{
			auto e_item = Entity::create();
			e_list->add_child(e_item);
			{
				e_item->add_component(cElement::create());

				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->set_text(L"item" + std::to_wstring(i));
				e_item->add_component(c_text);

				e_item->add_component(cEventReceiver::create());

				e_item->add_component(cStyleBackgroundColor::create(default_style.frame_color_normal, default_style.frame_color_hovering, default_style.frame_color_active));

				e_item->add_component(cListItem::create());

				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeFitParent;
				e_item->add_component(c_aligner);
			}
		}

		auto e_container = wrap_standard_scrollbar(e_list, ScrollbarVertical, false, 1.f);
		e_layout_right->add_child(e_container);
		{
			auto c_element = (cElement*)e_container->find_component(cH("Element"));
			c_element->width = 200.f;
			c_element->height = 100.f;
			c_element->inner_padding = Vec4f(4.f);
			c_element->background_frame_color = Vec4c(255);
			c_element->background_frame_thickness = 2.f;
			c_element->clip_children = true;
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
			((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
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
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Add %s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Add", app.root, e_menu, true, SideE, false, Icon_CARET_RIGHT);
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
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("Remove %s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Remove", app.root, e_menu, true, SideE, false, Icon_CARET_RIGHT);
			e_popup_menu->add_child(e_menu_btn);
		}
	}

	{
		struct Data
		{
			Entity* menu;
			Entity* root;
		}data;
		data.menu = e_popup_menu;
		data.root = app.root;
		app.c_event_receiver_root->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
			if (is_mouse_down(action, key, true) && key == Mouse_Right)
			{
				auto data = (Data*)c;
				popup_menu(data->menu, data->root, pos);
			}
		}, new_mail(&data));
	}
	
	auto e_menubar = Entity::create();
	app.root->add_child(e_menubar);
	{
		auto c_element = cElement::create();
		c_element->background_color = default_style.frame_color_normal;
		e_menubar->add_component(c_element);

		auto c_aligner = cAligner::create();
		c_aligner->width_policy = SizeFitParent;
		e_menubar->add_component(c_aligner);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutHorizontal;
		c_layout->item_padding = 4.f;
		e_menubar->add_component(c_layout);
	}

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
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"File", app.root, e_menu, true, SideS, true, nullptr);
			((cAligner*)e_menu_btn->find_component(cH("Aligner")))->width_policy = SizeFixed;
			((cStyleBackgroundColor*)e_menu_btn->find_component(cH("StyleBackgroundColor")))->color_normal.w() = 0;
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
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Edit", app.root, e_menu, true, SideS, true, nullptr);
			((cAligner*)e_menu_btn->find_component(cH("Aligner")))->width_policy = SizeFixed;
			((cStyleBackgroundColor*)e_menu_btn->find_component(cH("StyleBackgroundColor")))->color_normal.w() = 0;
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
				((cEventReceiver*)e_item->find_component(cH("EventReceiver")))->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
					{
						printf("%s!\n", *(char**)c);
						destroy_topmost(app.root);
					}
				}, new_mail_p((char*)names[i]));
			}
			auto e_menu_btn = create_standard_menu_button(app.font_atlas_pixel, 1.f, L"Tool", app.root, e_menu, true, SideS, true, nullptr);
			((cAligner*)e_menu_btn->find_component(cH("Aligner")))->width_policy = SizeFixed;
			((cStyleBackgroundColor*)e_menu_btn->find_component(cH("StyleBackgroundColor")))->color_normal.w() = 0;
			e_menubar->add_child(e_menu_btn);
		}
	}

	auto e_combobox = create_standard_combobox(100.f, app.font_atlas_pixel, 1.f, app.root, { L"Apple", L"Boy", L"Cat" }, -1);
	e_layout_right->add_child(e_combobox);

	auto e_tree = Entity::create();
	e_layout_right->add_child(e_tree);
	{
		auto c_element = cElement::create();
		c_element->inner_padding = Vec4f(4.f);
		c_element->background_frame_color = Vec4c(255);
		c_element->background_frame_thickness = 2.f;
		e_tree->add_component(c_element);

		auto c_layout = cLayout::create();
		c_layout->type = LayoutVertical;
		c_layout->item_padding = 4.f;
		e_tree->add_component(c_layout);

		e_tree->add_component(cTree::create());
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
		app.root->add_child(e_container);
		{
			auto c_element = (cElement*)e_container->find_component(cH("Element"));
			c_element->x = 414.f;
			c_element->y = 297.f;
			c_element->width = 221.f;
			c_element->height = 214.f;
		}

		auto e_docker = get_docker_model()->copy();
		e_container->add_child(e_docker);
		auto e_tabbar = e_docker->child(0);
		auto e_pages = e_docker->child(1);

		for (auto i = 0; i < 1; i++)
		{
			static const wchar_t* names[] = {
				L"Hierarchy",
			};

			e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], app.root));

			auto e_page = get_docker_page_model()->copy();
			e_pages->add_child(e_page);
			{
				auto c_text = cText::create(app.font_atlas_pixel);
				c_text->auto_size = false;
				c_text->set_text(names[i]);
				e_page->add_component(c_text);
			}
		}
	}

	{
		auto e_container = get_docker_container_model()->copy();
		app.root->add_child(e_container);
		{
			auto c_element = (cElement*)e_container->find_component(cH("Element"));
			c_element->x = 667.f;
			c_element->y = 302.f;
			c_element->width = 403.f;
			c_element->height = 215.f;
		}

		auto e_docker_layout = get_docker_layout_model()->copy();
		e_container->add_child(e_docker_layout);

		{
			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 0);
				{
					auto c_aligner = (cAligner*)e_docker->find_component(cH("Aligner"));
					c_aligner->x_align = AlignxFree;
					c_aligner->y_align = AlignyFree;
					c_aligner->using_padding_in_free_layout = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					static const wchar_t* names[] = {
						L"Inspector",
						L"ResourceExplorer"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], app.root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_size = false;
						c_text->set_text(names[i]);
						e_page->add_component(c_text);
					}
				}
			}

			{
				auto e_docker = get_docker_model()->copy();
				e_docker_layout->add_child(e_docker, 2);
				{
					auto c_aligner = (cAligner*)e_docker->find_component(cH("Aligner"));
					c_aligner->x_align = AlignxFree;
					c_aligner->y_align = AlignyFree;
					c_aligner->using_padding_in_free_layout = false;
				}

				auto e_tabbar = e_docker->child(0);
				auto e_pages = e_docker->child(1);

				for (auto i = 0; i < 2; i++)
				{
					static const wchar_t* names[] = {
						L"TextEditor",
						L"ShaderEditor"
					};

					e_tabbar->add_child(create_standard_docker_tab(app.font_atlas_pixel, names[i], app.root));

					auto e_page = get_docker_page_model()->copy();
					e_pages->add_child(e_page);
					{
						auto c_text = cText::create(app.font_atlas_pixel);
						c_text->auto_size = false;
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
