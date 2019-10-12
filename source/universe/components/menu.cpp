#include <flame/graphics/font.h>
#include <flame/universe/default_style.h>
#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>

namespace flame
{
	struct cMenuButtonPrivate : cMenuButton
	{
		void* mouse_listener;

		cMenuButtonPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			root = nullptr;
			menu = nullptr;
			move_to_open = true;
			popup_side = SideE;
			topmost_penetrable = false;

			opened = false;

			mouse_listener = nullptr;
		}

		~cMenuButtonPrivate()
		{
			if (!entity->dying)
				event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));

			if (event_receiver)
			{
				mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cMenuButtonPrivate**)c;
					if (thiz->can_open(action, key))
						thiz->open();
				}, new_mail_p(this));
			}
		}

		void open()
		{
			if (!opened)
			{
				if (entity->parent())
					close_menu(entity->parent());

				opened = true;

				auto topmost = get_topmost(root);
				if (!topmost)
					topmost = create_topmost(root, topmost_penetrable, true, !move_to_open);
				else
					topmost->created_frame = looper().frame;

				auto c_menu = (cMenu*)menu->find_component(cH("Menu"));
				if (c_menu)
					c_menu->popuped_by = this;
				auto menu_element = (cElement*)menu->find_component(cH("Element"));
				switch (popup_side)
				{
				case SideS:
					menu_element->pos.x() = element->global_pos.x();
					menu_element->pos.y() = element->global_pos.y() + element->global_size.y();
					break;
				case SideE:
					menu_element->pos.x() = element->global_pos.x() + element->global_size.x();
					menu_element->pos.y() = element->global_pos.y();
					break;
				}
				menu_element->scale = element->global_scale;

				topmost->add_child(menu);
			}
		}

		void close()
		{
			if (opened)
			{
				opened = false;

				close_menu(menu);

				get_topmost(root)->remove_child(menu, false);
			}
		}
	};

	bool cMenuButton::can_open(KeyState action, MouseKey key)
	{
		if ((is_mouse_down(action, key, true) && key == Mouse_Left))
			return true;
		else if (move_to_open && is_mouse_move(action, key))
		{
			auto t = get_topmost(root);
			if (t && t->name_hash() == cH("topmost"))
				return true;
		}
		return false;
	}

	void cMenuButton::start()
	{
		((cMenuButtonPrivate*)this)->start();
	}

	void cMenuButton::update()
	{
	}

	void cMenuButton::open()
	{
		((cMenuButtonPrivate*)this)->open();
	}

	void cMenuButton::close()
	{
		((cMenuButtonPrivate*)this)->close();
	}

	cMenuButton* cMenuButton::create()
	{
		return new cMenuButtonPrivate();
	}

	struct cMenuPrivate : cMenu
	{
		cMenuPrivate()
		{
			popuped_by = nullptr;
		}
	};

	void cMenu::update()
	{
	}

	cMenu* cMenu::create()
	{
		return new cMenuPrivate();
	}

	void close_menu(Entity* menu)
	{
		for (auto i = 0; i < menu->child_count(); i++)
		{
			auto menu_btn = (cMenuButton*)menu->child(i)->find_component(cH("MenuButton"));
			if (menu_btn)
				menu_btn->close();
		}
	}

	void popup_menu(Entity* menu, Entity* root, const Vec2f& pos)
	{
		auto topmost = get_topmost(root);
		if (!topmost)
			topmost = create_topmost(root, false, true, false);

		((cElement*)(menu->find_component(cH("Element"))))->pos = pos;

		topmost->add_child(menu);
	}

	Entity* create_standard_menu()
	{
		auto e_menu = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->color = default_style.window_color;
			e_menu->add_component(c_element);

			e_menu->add_component(cLayout::create(LayoutVertical));

			e_menu->add_component(cMenu::create());
		}

		return e_menu;
	}

	Entity* create_standard_menu_item(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text)
	{
		auto e_item = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_item->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			c_text->set_text(text);
			e_item->add_component(c_text);

			e_item->add_component(cEventReceiver::create());

			e_item->add_component(cStyleColor::create(default_style.frame_color_normal, default_style.frame_color_hovering, default_style.frame_color_active));

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeGreedy;
			e_item->add_component(c_aligner);
		}

		return e_item;
	}

	Entity* create_standard_menu_button(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text, Entity* root, Entity* menu, bool move_to_open, Side popup_side, bool topmost_penetrable, bool width_greedy, bool background_transparent, const wchar_t* arrow_text)
	{
		auto e_menu_btn = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding = Vec4f(4.f, 2.f, 4.f + (arrow_text ? font_atlas->pixel_height * sdf_scale : 0.f), 2.f);
			e_menu_btn->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->sdf_scale = sdf_scale;
			if (text[0])
				c_text->set_text(text);
			e_menu_btn->add_component(c_text);

			e_menu_btn->add_component(cEventReceiver::create());

			auto c_menu_btn = cMenuButton::create();
			c_menu_btn->root = root;
			c_menu_btn->menu = menu;
			c_menu_btn->move_to_open = move_to_open;
			c_menu_btn->popup_side = popup_side;
			c_menu_btn->topmost_penetrable = topmost_penetrable;
			e_menu_btn->add_component(c_menu_btn);

			e_menu_btn->add_component(cStyleColor::create(background_transparent ? Vec4c(0) : default_style.frame_color_normal, default_style.frame_color_hovering, default_style.frame_color_active));

			if (width_greedy)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy = SizeGreedy;
				e_menu_btn->add_component(c_aligner);
			}

			if (arrow_text)
			{
				e_menu_btn->add_component(cLayout::create(LayoutFree));

				auto e_arrow = Entity::create();
				e_menu_btn->add_child(e_arrow);
				{
					auto c_element = cElement::create();
					c_element->inner_padding = Vec4f(0.f, 2.f, 4.f, 2.f);
					e_arrow->add_component(c_element);

					auto c_text = cText::create(font_atlas);
					c_text->sdf_scale = sdf_scale;
					c_text->set_text(arrow_text);
					e_arrow->add_component(c_text);

					auto c_aligner = cAligner::create();
					c_aligner->x_align = AlignxRight;
					e_arrow->add_component(c_aligner);
				}
			}
		}

		return e_menu_btn;
	}

	Entity* create_standard_menubar()
	{
		auto e_menubar = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->color = default_style.frame_color_normal;
			e_menubar->add_component(c_element);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy = SizeFitParent;
			e_menubar->add_component(c_aligner);

			auto c_layout = cLayout::create(LayoutHorizontal);
			c_layout->item_padding = 4.f;
			e_menubar->add_component(c_layout);
		}

		return e_menubar;
	}
}
