#include <flame/universe/world.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/components/element.h>
#include "event_receiver_private.h"

namespace flame
{
	cEventReceiverPrivate::cEventReceiverPrivate()
	{
		dispatcher = nullptr;
		element = nullptr;

		pass = nullptr;
		accept_key = false;
		drag_hash = 0;

		hovering = false;
		focusing = false;
		active = false;
		dragging = false;
		state = EventReceiverNormal;

		key_listeners.impl = ListenerHubImpl::create();
		mouse_listeners.impl = ListenerHubImpl::create();
		drag_and_drop_listeners.impl = ListenerHubImpl::create();
	}

	cEventReceiverPrivate::~cEventReceiverPrivate()
	{
		ListenerHubImpl::destroy(key_listeners.impl);
		ListenerHubImpl::destroy(mouse_listeners.impl);
		ListenerHubImpl::destroy(drag_and_drop_listeners.impl);
	}

	void cEventReceiverPrivate::on_key(KeyStateFlags action, uint value)
	{
		key_listeners.call(action, value);
	}

	void cEventReceiverPrivate::on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value)
	{
		mouse_listeners.call(action, key, value);
	}

	void cEventReceiverPrivate::on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos)
	{
		drag_and_drop_listeners.call(action, er, pos);
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
		if (c->name_hash == FLAME_CHASH("cElement"))
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

		copy->pass = pass;
		copy->drag_hash = drag_hash;

		return copy;
	}

	void cEventReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
	{
		auto& drops = ((cEventReceiverPrivate*)this)->acceptable_drops;
		drops.resize(drop_count);
		for (auto i = 0; i < drop_count; i++)
			drops[i] = _drops[i];
	}

	cEventReceiver* cEventReceiver::create()
	{
		return new cEventReceiverPrivate();
	}
}
