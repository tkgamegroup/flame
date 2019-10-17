#include "../entity_private.h"
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
		state = EventReceiverNormal;

		drag_hash = 0;

		focus_listeners.hub = new ListenerHub;
		key_listeners.hub = new ListenerHub;
		mouse_listeners.hub = new ListenerHub;
		drag_and_drop_listeners.hub = new ListenerHub;
		state_changed_listeners.hub = new ListenerHub;
	}

	cEventReceiverPrivate::~cEventReceiverPrivate()
	{
		if (focusing)
			event_dispatcher->focusing = nullptr;
		if (hovering)
			event_dispatcher->hovering = nullptr;

		delete (ListenerHub*)focus_listeners.hub;
		delete (ListenerHub*)key_listeners.hub;
		delete (ListenerHub*)mouse_listeners.hub;
		delete (ListenerHub*)drag_and_drop_listeners.hub;
		delete (ListenerHub*)state_changed_listeners.hub;
	}

	void cEventReceiverPrivate::on_focus(FocusType type)
	{
		auto& listeners = ((ListenerHub*)focus_listeners.hub)->listeners;
		for (auto& l : listeners)
			((void(*)(void*, FocusType))l->function)(l->capture.p, type);
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

	void cEventReceiverPrivate::on_state_changed(EventReceiverState prev_state, EventReceiverState curr_state)
	{
		auto& listeners = ((ListenerHub*)state_changed_listeners.hub)->listeners;
		for (auto& l : listeners)
			((void(*)(void*, EventReceiverState prev_state, EventReceiverState curr_state))l->function)(l->capture.p, prev_state, curr_state);
	}

	void cEventReceiverPrivate::on_component_added(Component* c)
	{
		if (c->type_hash == cH("Element"))
			element = (cElement*)c;
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

	void cEventReceiver::on_component_added(Component* c)
	{
		((cEventReceiverPrivate*)this)->on_component_added(c);
	}

	Component* cEventReceiver::copy()
	{
		return ((cEventReceiverPrivate*)this)->copy();
	}

	cEventReceiver* cEventReceiver::create()
	{
		return new cEventReceiverPrivate();
	}
}
