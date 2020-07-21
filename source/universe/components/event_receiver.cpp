#include "../world_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "../systems/event_dispatcher_private.h"
//
namespace flame
{
//	cEventReceiverPrivate::cEventReceiverPrivate()
//	{
//		focus_type = FocusByLeftButton;
//		drag_hash = 0;
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
//	void cEventReceiverPrivate::on_hovering(bool hovering)
//	{
//		hover_listeners.call_with_current(this, hovering);
//	}
//
//	void cEventReceiverPrivate::on_focusing(bool focusing)
//	{
//		focus_listeners.call_with_current(this, focusing);
//	}

	void cEventReceiverPrivate::on_entered_world()
	{
		dispatcher = (sEventDispatcherPrivate*)((EntityPrivate*)entity)->world->get_system(sEventDispatcherPrivate::type_hash);
		mark_dirty();
	}

	void cEventReceiverPrivate::on_left_world()
	{
		if (dispatcher)
		{
			if (this == dispatcher->hovering)
				dispatcher->hovering = nullptr;
			if (this == dispatcher->focusing)
			{
				dispatcher->focusing = nullptr;
				dispatcher->active = nullptr;
				dispatcher->dragging = nullptr;
			}
			if (this == dispatcher->key_target)
				dispatcher->key_target = nullptr;
			if (this == dispatcher->drag_overing)
				dispatcher->drag_overing = nullptr;
			//if (er == next_focusing)
			//	next_focusing = (cEventReceiver*)INVALID_POINTER;

			((EntityPrivate*)entity)->set_state((StateFlags)((int)((EntityPrivate*)entity)->state & (~StateHovering) & (~StateActive)));
		}
		mark_dirty();
		dispatcher = nullptr;
	}

	void cEventReceiverPrivate::on_entity_visibility_changed()
	{
		mark_dirty();
	}

	void cEventReceiverPrivate::mark_dirty() 
	{
		if (dispatcher)
			dispatcher->dirty = true;
	}

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
		return f_new<cEventReceiverPrivate>();
	}

	cEventReceiver* cEventReceiver::create() { return cEventReceiverPrivate::create(); }
}
