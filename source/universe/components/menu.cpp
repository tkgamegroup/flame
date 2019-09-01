#include <flame/universe/topmost.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
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
			event_receiver->remove_mouse_listener(mouse_listener);
		}

		void start()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));

			if (event_receiver)
			{
				mouse_listener = event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					auto thiz = *(cMenuButtonPrivate**)c;
					if ((is_mouse_down(action, key, true) && key == Mouse_Left) || (thiz->move_to_open && is_mouse_move(action, key) && get_topmost()))
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

				auto topmost = get_topmost();
				if (!topmost)
					topmost = create_topmost(root, topmost_penetrable);
				else
					topmost->created_frame = looper().frame;

				auto c_menu = (cMenu*)menu->find_component(cH("Menu"));
				if (c_menu)
					c_menu->popuped_by = this;
				auto menu_element = (cElement*)menu->find_component(cH("Element"));
				if (popup_side == SideE)
				{
					menu_element->x = element->global_x + element->global_width;
					menu_element->y = element->global_y;
				}
				else if (popup_side == SideS)
				{
					menu_element->x = element->global_x;
					menu_element->y = element->global_y + element->global_height;
				}

				topmost->add_child(menu);
			}
		}

		void close()
		{
			if (opened)
			{
				opened = false;

				close_menu(menu);

				get_topmost()->take_child(menu);
			}
		}
	};

	cMenuButton::~cMenuButton()
	{
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

	cMenu::~cMenu()
	{
	}

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
		auto topmost = get_topmost();
		if (!topmost)
			topmost = create_topmost(root);

		auto element = (cElement*)(menu->find_component(cH("Element")));
		element->x = pos.x();
		element->y = pos.y();

		topmost->add_child(menu);
	}
}
