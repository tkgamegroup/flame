#include <flame/universe/utils.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/menu.h>

namespace flame
{
	struct cSubMenuPrivate : cSubMenu
	{
		cSubMenuPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			items = nullptr;

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
						(*(cSubMenuPrivate**)c)->open();

				}, new_mail_p(this));
			}
		}

		void open()
		{
			if (!opened)
			{
				opened = true;

				auto topmost = get_topmost();

				auto items_element = (cElement*)items->find_component(cH("Element"));
				items_element->x = element->global_x + element->global_width;
				items_element->y = element->global_y;

				topmost->add_child(items);

				auto parent = entity->parent();
				if (parent)
				{
					for (auto i = 0; i < parent->child_count(); i++)
					{
						auto e = parent->child(i);
						if (e != entity)
						{
							auto menu = (cSubMenu*)e->find_component(cH("SubMenu"));
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
				topmost->take_child(items);

				for (auto i = 0; i < items->child_count(); i++)
				{
					auto e = items->child(i);
					auto menu = (cSubMenu*)e->find_component(cH("Menu"));
					if (menu)
						menu->close();
				}
			}
		}
	};

	cSubMenu::~cSubMenu()
	{
	}

	void cSubMenu::on_add_to_parent()
	{
		((cSubMenuPrivate*)this)->on_add_to_parent();
	}

	void cSubMenu::update()
	{
	}

	void cSubMenu::open()
	{
		((cSubMenuPrivate*)this)->open();
	}

	void cSubMenu::close()
	{
		((cSubMenuPrivate*)this)->close();
	}

	cSubMenu* cSubMenu::create()
	{
		return new cSubMenuPrivate();
	}

	struct cPopupMenuPrivate : cPopupMenu
	{
		cPopupMenuPrivate()
		{
			element = nullptr;
		}

		void on_add_to_parent()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
		}

		void open(Entity* root, const Vec2f& pos)
		{
			auto topmost = get_topmost();
			if (!topmost)
				topmost = create_topmost(root);

			element->x = pos.x();
			element->y = pos.y();

			topmost->add_child(entity);
		}
	};

	cPopupMenu::~cPopupMenu()
	{
	}

	void cPopupMenu::on_add_to_parent()
	{
		((cPopupMenuPrivate*)this)->on_add_to_parent();
	}

	void cPopupMenu::update()
	{
	}

	void cPopupMenu::open(Entity* root, const Vec2f& pos)
	{
		((cPopupMenuPrivate*)this)->open(root, pos);
	}

	cPopupMenu* cPopupMenu::create()
	{
		return new cPopupMenuPrivate();
	}
}
