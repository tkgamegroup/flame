#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		//Array<Function<FoucusListenerParm>> focus_listeners$;
		//Array<Function<KeyListenerParm>> key_listeners$;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, MouseKey key, const Vec2f& pos)>>> mouse_listeners;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cEventReceiverPrivate(Entity* e) :
			cEventReceiver(e)
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);

			hovering = false;
			dragging = false;
			focusing = false;
		}

		void update()
		{
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

	void* cEventReceiver::add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2f& pos), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, KeyState action, MouseKey key, const Vec2f & pos)>;
		c->function = listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->mouse_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiver::remove_mouse_listener(void* ret_by_add)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->mouse_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
	}

	void cEventReceiver::on_mouse(KeyState action, MouseKey key, const Vec2f& value)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->mouse_listeners;
		for (auto& l : listeners)
			l->function(l->capture.p, action, key, value);
	}

	cEventReceiver* cEventReceiver::create(Entity* e)
	{
		return new cEventReceiverPrivate(e);
	}
}
