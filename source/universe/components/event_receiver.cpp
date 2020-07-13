#include "../world_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
//
namespace flame
{
//	cEventReceiverPrivate::cEventReceiverPrivate()
//	{
//		dispatcher = nullptr;
//		element = nullptr;
//
//		focus_type = FocusByLeftButton;
//		drag_hash = 0;
//		state = EventReceiverNormal;
//
//		mouse_listeners.add([](Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos) {
//			if (is_mouse_clicked(action, key))
//			{
//				auto thiz = c.thiz<cEventReceiverPrivate>();
//				thiz->clicked_listeners.call_no_check_with_current(thiz);
//			}
//			return true;
//		}, Capture().set_thiz(this));
//
//		frame = -1;
//	}

//	void cEventReceiverPrivate::on_key(KeyStateFlags action, uint value)
//	{
//		key_listeners.call_with_current(this, action, value);
//	}

	void cEventReceiverPrivate::on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value)
	{
		for (auto& l : mouse_listeners)
		{
			if (!l->call(action, key, value))
				break;
		}
	}

//	void cEventReceiverPrivate::on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos)
//	{
//		drag_and_drop_listeners.call_with_current(this, action, er, pos);
//	}
//
//	void cEventReceiverPrivate::set_state(EventReceiverState _state)
//	{
//		if (state != _state)
//		{
//			state = _state;
//			state_listeners.call_with_current(this, state);
//		}
//	}
//
//	void cEventReceiverPrivate::on_hovering(bool hovering)
//	{
//		hover_listeners.call_with_current(this, hovering);
//	}
//
//	void cEventReceiverPrivate::on_focusing(bool focusing)
//	{
//		focus_listeners.call_with_current(this, focusing);
//	}
//
//	void cEventReceiverPrivate::on_event(EntityEvent e, void* t)
//	{
//		switch (e)
//		{
//		case EntityEnteredWorld:
//			dispatcher = entity->world->get_system(sEventDispatcher);
//			dispatcher->pending_update = true;
//			break;
//		case EntityLeftWorld:
//			((sEventDispatcherPrivate*)dispatcher)->on_receiver_removed(this);
//			dispatcher->pending_update = true;
//			dispatcher = nullptr;
//			break;
//		case EntityComponentAdded:
//			if (t == this)
//			{
//				element = entity->get_component(cElement);
//				assert(element);
//			}
//			break;
//		case EntityVisibilityChanged:
//			if (dispatcher)
//				dispatcher->pending_update = true;
//			break;
//		}
//	}
//
//	void cEventReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
//	{
//		auto& drops = ((cEventReceiverPrivate*)this)->acceptable_drops;
//		drops.resize(drop_count);
//		for (auto i = 0; i < drop_count; i++)
//			drops[i] = _drops[i];
//	}
//
//	void cEventReceiver::on_key(KeyStateFlags action, uint value)
//	{
//		((cEventReceiverPrivate*)this)->on_key(action, value);
//	}
//
//	void cEventReceiver::on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value)
//	{
//		((cEventReceiverPrivate*)this)->on_mouse(action, key, value);
//	}
//
//	void cEventReceiver::on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos)
//	{
//		((cEventReceiverPrivate*)this)->on_drag_and_drop(action, er, pos);
//	}

	void cEventReceiverPrivate::on_added() 
	{
		element = (cElementPrivate*)((EntityPrivate*)entity)->get_component(cElement::type_hash);
	}

	void* cEventReceiverPrivate::add_mouse_listener(bool (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_listener(void* lis)
	{
		std::erase_if(mouse_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	cEventReceiverPrivate* cEventReceiverPrivate::create()
	{
		auto ret = _allocate(sizeof(cEventReceiverPrivate));
		new (ret) cEventReceiverPrivate;
		return (cEventReceiverPrivate*)ret;
	}

	cEventReceiver* cEventReceiver::create() { return cEventReceiverPrivate::create(); }
}
