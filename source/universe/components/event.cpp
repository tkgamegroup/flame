#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event.h>

namespace flame
{
	struct cEventPrivate : cEvent
	{
		cElement* element;

		//Array<Function<FoucusListenerParm>> focus_listeners$;
		//Array<Function<KeyListenerParm>> key_listeners$;
		//Array<Function<MouseListenerParm>> mouse_listeners$;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cEventPrivate(Entity* e) :
			cEvent(e),
			element(nullptr)
		{
			blackhole = false;
			want_key = false;

			hovering = false;
			dragging = false;
			focusing = false;

			element = (cElement*)(entity->component(cH("Element")));
			assert(element);
		}

		void update()
		{
		}

		bool contains(const Vec2f& pos) const
		{
			return rect_contains(Vec4f(element->global_x, element->global_y, element->global_width, element->global_height), pos);
		}
	};

	cEvent::cEvent(Entity* e) :
		Component("Event", e)
	{
	}

	cEvent::~cEvent()
	{
	}

	void cEvent::update()
	{
		((cEventPrivate*)this)->update();
	}

	bool cEvent::contains(const Vec2f& pos) const
	{
		return ((cEventPrivate*)this)->contains(pos);
	}

	cEvent* cEvent::create(Entity* e)
	{
		return new cEventPrivate(e);
	}
}
