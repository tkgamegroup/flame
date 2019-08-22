#include <flame/universe/utils.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>

namespace flame
{

	Entity* create_topmost(Entity* e, bool close_when_clicked)
	{
		auto topmost = Entity::create();
		topmost->set_name("topmost");
		e->add_child(topmost);

		{
			auto e_c_element = (cElement*)e->find_component(cH("Element"));
			assert(e_c_element);

			auto c_element = cElement::create();
			c_element->width = e_c_element->width;
			c_element->height = e_c_element->height;
			topmost->add_component(c_element);

			if (close_when_clicked)
			{
				auto c_event_receiver = cEventReceiver::create();
				c_event_receiver->add_mouse_listener([](void* c, KeyState action, MouseKey key, const Vec2f& pos) {
					if (is_mouse_down(action, key, true) && key == Mouse_Left)
						destroy_topmost(*(Entity**)c);
				}, new_mail_p(e));
				topmost->add_component(c_event_receiver);
			}
		}

		return topmost;
	}

	void destroy_topmost(Entity* e)
	{
		auto topmost = e->find_child("topmost");
		topmost->take_all_children();
		e->remove_child(topmost);
	}
}
