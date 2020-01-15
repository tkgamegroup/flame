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
#include <flame/universe/ui/make_menu.h>

namespace flame
{
	struct cMenuPrivate : cMenu
	{
		cMenuPrivate()
		{
			button = nullptr;
		}
	};

	cMenu* cMenu::create()
	{
		return new cMenuPrivate();
	}

	struct cMenuButtonPrivate : cMenuButton
	{
		void* mouse_listener;

		cMenuButtonPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			root = nullptr;
			menu = Entity::create();
			make_menu(menu);
			menu->get_component(cMenu)->button = this;
			popup_side = SideE;
			move_to_open = true;
			layer_penetrable = false;

			opened = false;

			mouse_listener = nullptr;
		}

		~cMenuButtonPrivate()
		{
			if (!entity->dying_)
				event_receiver->mouse_listeners.remove(mouse_listener);
			Entity::destroy(menu);
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
			layer = ui::add_layer(root, false, true, true);

		menu->get_component(cElement)->set_pos(pos);

		layer->add_child(menu);
	}
}
