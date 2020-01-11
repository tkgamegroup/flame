#include <flame/graphics/font.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/text.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>
#include <flame/universe/components/aligner.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/components/menu.h>
#include <flame/universe/ui/layer.h>
#include <flame/universe/ui/style_stack.h>

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
			layer_penetrable = false;

			opened = false;

			mouse_listener = nullptr;
		}

		~cMenuButtonPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
		}

		void open()
		{
			if (!opened)
			{
				if (entity->parent())
					close_menu(entity->parent());

				opened = true;

				auto layer = ui::get_top_layer(root);
				if (!layer)
					layer = ui::add_layer(root, layer_penetrable, true, move_to_open);
				else
					layer->created_frame_ = looper().frame;

				auto c_menu = menu->get_component(cMenu);
				if (c_menu)
					c_menu->popuped_by = this;
				auto menu_element = menu->get_component(cElement);
				switch (popup_side)
				{
				case SideS:
					menu_element->set_x(element->global_pos.x());
					menu_element->set_y(element->global_pos.y() + element->global_size.y());
					break;
				case SideE:
					menu_element->set_x(element->global_pos.x() + element->global_size.x());
					menu_element->set_y(element->global_pos.y());
					break;
				}
				menu_element->set_scale(element->global_scale);

				layer->add_child(menu);
			}
		}

		void close()
		{
			if (opened)
			{
				opened = false;

				close_menu(menu);

				ui::get_top_layer(root)->remove_child(menu, false);
			}
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
				element = (cElement*)c;
			else if (c->name_hash == FLAME_CHASH("cEventReceiver"))
			{
				event_receiver = (cEventReceiver*)c;
				mouse_listener = event_receiver->mouse_listeners.add([](void* c, KeyState action, MouseKey key, const Vec2i& pos) {
					auto thiz = *(cMenuButtonPrivate**)c;
					if (thiz->can_open(action, key))
						thiz->open();
				}, new_mail_p(this));
			}
		}
	};

	bool cMenuButton::can_open(KeyState action, MouseKey key)
	{
		if ((is_mouse_down(action, key, true) && key == Mouse_Left))
			return true;
		else if (move_to_open && is_mouse_move(action, key))
		{
			auto l = ui::get_top_layer(root);
			if (l && l->name_hash() == FLAME_CHASH("layer_mmto"))
				return true;
		}
		return false;
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

	cMenu* cMenu::create()
	{
		return new cMenuPrivate();
	}

	void close_menu(Entity* menu)
	{
		for (auto i = 0; i < menu->child_count(); i++)
		{
			auto menu_btn = menu->child(i)->get_component(cMenuButton);
			if (menu_btn)
				menu_btn->close();
		}
	}

	void popup_menu(Entity* menu, Entity* root, const Vec2f& pos)
	{
		auto layer = ui::get_top_layer(root);
		if (!layer)
			layer = ui::add_layer(root, false, true, false);

		menu->get_component(cElement)->set_pos(pos);

		layer->add_child(menu);
	}

	Entity* create_standard_menu()
	{
		auto e_menu = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->color_ = ui::style(ui::WindowColor).c();
			e_menu->add_component(c_element);

			e_menu->add_component(cLayout::create(LayoutVertical));

			e_menu->add_component(cMenu::create());
		}

		return e_menu;
	}

	Entity* create_standard_menu_item(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text)
	{
		auto e_item = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f, 2.f);
			e_item->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = ui::style(ui::FontSize).u()[0] * font_size_scale;
			c_text->set_text(text);
			e_item->add_component(c_text);

			e_item->add_component(cEventReceiver::create());

			auto c_style = cStyleColor::create();
			c_style->color_normal = ui::style(ui::FrameColorNormal).c();
			c_style->color_hovering = ui::style(ui::FrameColorHovering).c();
			c_style->color_active = ui::style(ui::FrameColorActive).c();
			e_item->add_component(c_style);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy_ = SizeGreedy;
			e_item->add_component(c_aligner);
		}

		return e_item;
	}

	Entity* create_standard_menu_button(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text, Entity* root, Entity* menu, bool move_to_open, Side popup_side, bool layer_penetrable, bool width_greedy, bool background_transparent, const wchar_t* arrow_text)
	{
		auto e_menu_btn = Entity::create();
		{
			auto c_element = cElement::create();
			c_element->inner_padding_ = Vec4f(4.f, 2.f, 4.f + (arrow_text ? ui::style(ui::FontSize).u()[0] * font_size_scale : 0.f), 2.f);
			e_menu_btn->add_component(c_element);

			auto c_text = cText::create(font_atlas);
			c_text->font_size_ = ui::style(ui::FontSize).u()[0] * font_size_scale;
			if (text[0])
				c_text->set_text(text);
			e_menu_btn->add_component(c_text);

			e_menu_btn->add_component(cEventReceiver::create());

			auto c_menu_btn = cMenuButton::create();
			c_menu_btn->root = root;
			c_menu_btn->menu = menu;
			c_menu_btn->popup_side = popup_side;
			c_menu_btn->layer_penetrable = layer_penetrable;
			c_menu_btn->move_to_open = move_to_open;
			e_menu_btn->add_component(c_menu_btn);

			auto c_style = cStyleColor::create();
			c_style->color_normal = background_transparent ? Vec4c(0) : ui::style(ui::FrameColorNormal).c();
			c_style->color_hovering = ui::style(ui::FrameColorHovering).c();
			c_style->color_active = ui::style(ui::FrameColorActive).c();
			e_menu_btn->add_component(c_style);

			if (width_greedy)
			{
				auto c_aligner = cAligner::create();
				c_aligner->width_policy_ = SizeGreedy;
				e_menu_btn->add_component(c_aligner);
			}

			if (arrow_text)
			{
				e_menu_btn->add_component(cLayout::create(LayoutFree));

				auto e_arrow = Entity::create();
				e_menu_btn->add_child(e_arrow);
				{
					auto c_element = cElement::create();
					c_element->inner_padding_ = Vec4f(0.f, 2.f, 4.f, 2.f);
					e_arrow->add_component(c_element);

					auto c_text = cText::create(font_atlas);
					c_text->font_size_ = ui::style(ui::FontSize).u()[0] * font_size_scale;
					c_text->set_text(arrow_text);
					e_arrow->add_component(c_text);

					auto c_aligner = cAligner::create();
					c_aligner->x_align_ = AlignxRight;
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
			c_element->color_ = ui::style(ui::FrameColorNormal).c();
			e_menubar->add_component(c_element);

			auto c_aligner = cAligner::create();
			c_aligner->width_policy_ = SizeFitParent;
			e_menubar->add_component(c_aligner);

			auto c_layout = cLayout::create(LayoutHorizontal);
			c_layout->item_padding = 4.f;
			e_menubar->add_component(c_layout);
		}

		return e_menubar;
	}
}
