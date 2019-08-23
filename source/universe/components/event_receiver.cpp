#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/event_dispatcher.h>

namespace flame
{
	struct cEventReceiverPrivate : cEventReceiver
	{
		//Array<Function<FoucusListenerParm>> focus_listeners$;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, uint value)>>> key_listeners;
		std::vector<std::unique_ptr<Closure<void(void* c, KeyState action, MouseKey key, const Vec2f& pos)>>> mouse_listeners;
		//Array<Function<DropListenerParm>> drop_listeners$;
		//Array<Function<ChangedListenerParm>> changed_listeners$;
		//Array<Function<ChildListenerParm>> child_listeners$;

		cEventReceiverPrivate()
		{
			element = nullptr;
			event_dispatcher = nullptr;

			penetrable = false;

			hovering = false;
			dragging = false;
			focusing = false;
		}

		void on_add_to_parent()
		{
			element = (cElement*)(entity->find_component(cH("Element")));
			assert(element);
		}

		void update()
		{
		}
	};

	cEventReceiver::~cEventReceiver()
	{
		if (focusing)
			event_dispatcher->focusing = nullptr;
		if (hovering)
			event_dispatcher->hovering = nullptr;
	}

	void cEventReceiver::on_add_to_parent()
	{
		((cEventReceiverPrivate*)this)->on_add_to_parent();
	}

	void cEventReceiver::update()
	{
		((cEventReceiverPrivate*)this)->update();
	}

	void* cEventReceiver::add_key_listener(void (*listener)(void* c, KeyState action, uint value), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, KeyState action, uint value)>;
		c->function = listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->key_listeners.emplace_back(c);
		return c;
	}

	void* cEventReceiver::add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2f& pos), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, KeyState action, MouseKey key, const Vec2f & pos)>;
		c->function 
		= listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->mouse_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiver::remove_key_listener(void* ret_by_add)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->key_listeners;
		for (auto it = listeners.begin(); it != listeners.end(); it++)
		{
			if (it->get() == ret_by_add)
			{
				listeners.erase(it);
				return;
			}
		}
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

	void cEventReceiver::on_key(KeyState action, uint value)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->key_listeners;
		for (auto& l : listeners)
			l->function(l->capture.p, action, value);
	}

	void cEventReceiver::on_mouse(KeyState action, MouseKey key, const Vec2f& value)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->mouse_listeners;
		for (auto& l : listeners)
			l->function(l->capture.p, action, key, value);
	}

	cEventReceiver* cEventReceiver::create()
	{
		return new cEventReceiverPrivate();
	}
}
