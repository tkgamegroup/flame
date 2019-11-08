#include "../universe_private.h"
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include "event_receiver_private.h"

namespace flame
{
	cEventReceiverPrivate::cEventReceiverPrivate()
	{
		dispatcher = nullptr;
		element = nullptr;

		penetrable = false;

		hovering = false;
		focusing = false;
		active = false;
		dragging = false;
		state = EventReceiverNormal;

		drag_hash = 0;

		key_listeners.hub = new ListenerHub;
		mouse_listeners.hub = new ListenerHub;
		drag_and_drop_listeners.hub = new ListenerHub;
	}

	cEventReceiverPrivate::~cEventReceiverPrivate()
	{
		delete (ListenerHub*)key_listeners.hub;
		delete (ListenerHub*)mouse_listeners.hub;
		delete (ListenerHub*)drag_and_drop_listeners.hub;
	}

	void cEventReceiverPrivate::on_key(KeyState action, uint value)
	{
		auto& listeners = ((ListenerHub*)key_listeners.hub)->listeners;
		for (auto& l : listeners)
			((void(*)(void*, KeyState action, int value))l->function)(l->capture.p, action, value);
	}

	void cEventReceiverPrivate::on_mouse(KeyState action, MouseKey key, const Vec2i& value)
	{
		auto& listeners = ((ListenerHub*)mouse_listeners.hub)->listeners;
		for (auto& l : listeners)
			((void(*)(void*, KeyState action, MouseKey key, const Vec2i & pos))l->function)(l->capture.p, action, key, value);
	}

	void cEventReceiverPrivate::on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos)
	{
		auto& listeners = ((ListenerHub*)drag_and_drop_listeners.hub)->listeners;
		for (auto& l : listeners)
			((void(*)(void*, DragAndDrop action, cEventReceiver * er, const Vec2i & pos))l->function)(l->capture.p, action, er, pos);
	}

	void cEventReceiverPrivate::on_entered_world()
	{
		dispatcher = entity->world_->get_system(sEventDispatcher);
		dispatcher->pending_update = true;
	}

	void cEventReceiverPrivate::on_left_world()
	{
		if (dispatcher)
		{
			dispatcher->receiver_leave_world(this);
			dispatcher->pending_update = true;
			dispatcher = nullptr;
		}
	}

	void cEventReceiverPrivate::on_component_added(Component* c)
	{
		if (c->name_hash == cH("cElement"))
			element = (cElement*)c;
	}

	void cEventReceiverPrivate::on_visibility_changed()
	{
		if (dispatcher)
			dispatcher->pending_update = true;
	}

	Component* cEventReceiverPrivate::copy()
	{
		auto copy = new cEventReceiverPrivate();

		copy->penetrable = penetrable;
		copy->drag_hash = drag_hash;

		return copy;
	}

	void cEventReceiver::set_acceptable_drops(const std::vector<uint>& hashes)
	{
		((cEventReceiverPrivate*)this)->acceptable_drops = hashes;
	}

	cEventReceiver* cEventReceiver::create()
	{
		return new cEventReceiverPrivate();
	}
}
