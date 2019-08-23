#include <flame/universe/utils.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/menu.h>

namespace flame
{
	struct cSubMenuButtonPrivate : cSubMenuButton
	{
		cSubMenuButtonPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			menu = nullptr;

			opened = false;
		}

		void on_add_to_parent()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));

			if (event_receiver)
			{
				event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						(*(cSubMenuButtonPrivate**)c)->open();

				}, new_mail_p(this));
			}
		}

		void open()
		{
			if (!opened)
			{
				opened = true;

				auto topmost = get_topmost();

				auto menu_element = (cElement*)menu->find_component(cH("Element"));
				menu_element->x = element->global_x + element->global_width;
				menu_element->y = element->global_y;

				topmost->add_child(menu);

				auto parent = entity->parent();
				if (parent)
				{
					for (auto i = 0; i < parent->child_count(); i++)
					{
						auto e = parent->child(i);
						if (e != entity)
						{
							auto menu = (cSubMenuButton*)e->find_component(cH("SubMenuButton"));
							if (menu)
								menu->close();
						}
					}
				}
			}
		}

		void close()
		{
			if (opened)
			{
				opened = false;

				auto topmost = get_topmost();
				topmost->take_child(menu);

				for (auto i = 0; i < menu->child_count(); i++)
				{
					auto e = menu->child(i);
					auto menu = (cSubMenuButton*)e->find_component(cH("SubMenuButton"));
					if (menu)
						menu->close();
				}
			}
		}
	};

	cSubMenuButton::~cSubMenuButton()
	{
	}

	void cSubMenuButton::on_add_to_parent()
	{
		((cSubMenuButtonPrivate*)this)->on_add_to_parent();
	}

	void cSubMenuButton::update()
	{
	}

	void cSubMenuButton::open()
	{
		((cSubMenuButtonPrivate*)this)->open();
	}

	void cSubMenuButton::close()
	{
		((cSubMenuButtonPrivate*)this)->close();
	}

	cSubMenuButton* cSubMenuButton::create()
	{
		return new cSubMenuButtonPrivate();
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
