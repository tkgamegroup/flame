#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/style.h>

namespace flame
{
	struct cStyleBgColPrivate : cStyleBgCol
	{
		cStyleBgColPrivate(const Vec4c& _col_normal, const Vec4c& _col_hovering, const Vec4c& _col_active)
		{
			element = nullptr;
			event_receiver = nullptr;

			col_normal = _col_normal;
			col_hovering = _col_hovering;
			col_active = _col_active;
		}

		void on_add_to_parent()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
			event_receiver = (cEventReceiver*)(entity->find_component(cH("EventReceiver")));
			assert(event_receiver);
		}

		void update()
		{
			if (event_receiver->dragging)
				element->background_color = col_active;
			else if (event_receiver->hovering)
				element->background_color = col_hovering;
			else
				element->background_color = col_normal;
		}
	};

	cStyleBgCol::~cStyleBgCol()
	{
	}

	void cStyleBgCol::on_add_to_parent()
	{
		((cStyleBgColPrivate*)this)->on_add_to_parent();
	}

	void cStyleBgCol::update()
	{
		((cStyleBgColPrivate*)this)->update();
	}

	cStyleBgCol* cStyleBgCol::create(const Vec4c& col_normal, const Vec4c& col_hovering, const Vec4c& col_active)
	{
		return new cStyleBgColPrivate(col_normal, col_hovering, col_active);
	}
}
