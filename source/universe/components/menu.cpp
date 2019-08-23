#include <flame/universe/utils.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/menu.h>

namespace flame
{
	struct cMenuPrivate : cMenu
	{
		cMenuPrivate()
		{
			element = nullptr;
			event_receiver = nullptr;

			root = nullptr;
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
					{
						auto thiz = *(cMenuPrivate**)c;
						thiz->open(Vec2f(-1.f));
					}

				}, new_mail_p(this));
			}
		}

		void update()
		{
		}

		void open(const Vec2f& pos)
		{
			if (!opened)
			{
				opened = true;

				auto topmost = get_topmost();
				if (!topmost)
					topmost = create_topmost(root);

				auto items_element = (cElement*)items->find_component(cH("Element"));
				if (pos.x() < 0.f && pos.y() < 0.f)
				{
					items_element->x = element->global_x + element->global_width;
					items_element->y = element->global_y;
				}
				else
				{
					items_element->x = pos.x();
					items_element->y = pos.y();
				}

				topmost->add_child(items);

				auto parent = entity->parent();
				if (parent)
				{
					for (auto i = 0; i < parent->child_count(); i++)
					{
						auto e = parent->child(i);
						if (e != entity)
						{
							auto menu = (cMenu*)e->find_component(cH("Menu"));
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
					auto menu = (cMenu*)e->find_component(cH("Menu"));
					if (menu)
						menu->close();
				}
			}
		}
	};

	cMenu::~cMenu()
	{
	}

	void cMenu::on_add_to_parent()
	{
		((cMenuPrivate*)this)->on_add_to_parent();
	}

	void cMenu::update()
	{
		((cMenuPrivate*)this)->update();
	}

	void cMenu::open(const Vec2f& pos)
	{
		((cMenuPrivate*)this)->open(pos);
	}

	void cMenu::close()
	{
		((cMenuPrivate*)this)->close();
	}

	cMenu* cMenu::create()
	{
		return new cMenuPrivate();
	}
}
