#include <flame/universe/components/element.h>
#include "event_receiver_private.h"
#include <flame/universe/components/event_dispatcher.h>

namespace flame
{
	cEventReceiverPrivate::cEventReceiverPrivate()
	{
		element = nullptr;
		event_dispatcher = nullptr;

		penetrable = false;

		hovering = false;
		focusing = false;
		active = false;
		dragging = false;

		drag_hash = 0;
	}

	cEventReceiverPrivate::~cEventReceiverPrivate()
	{
		if (focusing)
			event_dispatcher->focusing = nullptr;
		if (hovering)
			event_dispatcher->hovering = nullptr;
	}

	void cEventReceiverPrivate::start()
	{
		element = (cElement*)(entity->find_component(cH("Element")));
		assert(element);
	}

	cEventReceiver::~cEventReceiver()
	{
		((cEventReceiverPrivate*)this)->~cEventReceiverPrivate();
	}

	void cEventReceiver::start()
	{
		((cEventReceiverPrivate*)this)->start();
	}

	void cEventReceiver::update()
	{
	}

	void cEventReceiver::set_acceptable_drops(const std::vector<uint>& hashes)
	{
		((cEventReceiverPrivate*)this)->acceptable_drops = hashes;
	}

	void* cEventReceiver::add_key_listener(void (*listener)(void* c, KeyState action, uint value), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, KeyState action, uint value)>;
		c->function = listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->key_listeners.emplace_back(c);
		return c;
	}

	void* cEventReceiver::add_mouse_listener(void (*listener)(void* c, KeyState action, MouseKey key, const Vec2f& value), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, KeyState action, MouseKey key, const Vec2f & value)>;
		c->function = listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->mouse_listeners.emplace_back(c);
		return c;
	}

	void* cEventReceiver::add_drag_and_drop_listener(void (*listener)(void* c, DragAndDrop action, cEventReceiver* er, const Vec2f& pos), const Mail<>& capture)
	{
		auto c = new Closure<void(void* c, DragAndDrop action, cEventReceiver * er, const Vec2f & pos)>;
		c->function = listener;
		c->capture = capture;
		((cEventReceiverPrivate*)this)->drag_and_drop_listeners.emplace_back(c);
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

	void cEventReceiver::remove_drag_and_drop_listener(void* ret_by_add)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->drag_and_drop_listeners;
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

	void cEventReceiver::on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2f& pos)
	{
		auto& listeners = ((cEventReceiverPrivate*)this)->drag_and_drop_listeners;
		for (auto& l : listeners)
			l->function(l->capture.p, action, er, pos);
	}

	cEventReceiver* cEventReceiver::create()
	{
		return new cEventReceiverPrivate();
	}
}
