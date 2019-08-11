#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		cElement* element;

		//Array<Function<FoucusListenerParm>> focus_listeners$;
		//Array<Function<KeyListenerParm>> key_listeners$;
		//Array<Function<MouseListenerParm>> mouse_listeners$;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cEventReceiverPrivate(Entity* e) :
			cEventReceiver(e),
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

	cEventReceiver::cEventReceiver(Entity* e) :
		Component("EventReceiver", e)
	{
	}

	cEventReceiver::~cEventReceiver()
	{
	}

	void cEventReceiver::update()
	{
		((cEventReceiverPrivate*)this)->update();
	}

	bool cEventReceiver::contains(const Vec2f& pos) const
	{
		return ((cEventReceiverPrivate*)this)->contains(pos);
	}

	cEventReceiver* cEventReceiver::create(Entity* e)
	{
		return new cEventReceiverPrivate(e);
	}
}
