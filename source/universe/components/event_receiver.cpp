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
//	}

	void cEventReceiverPrivate::send_key_event(KeyStateFlags action, uint value)
	{
		for (auto& l : key_listeners)
		{
			if (!l->call(action, value))
				break;
		}
	}

	void cEventReceiverPrivate::send_mouse_event(KeyStateFlags action, MouseKey key, const Vec2i& value)
	{
		for (auto& l : mouse_listeners)
		{
			if (!l->call(action, key, value))
				break;
		}
	}

//	void cEventReceiverPrivate::send_drag_and_drop_event(DragAndDrop action, cEventReceiver* er, const Vec2i& pos)
//	{
//		drag_and_drop_listeners.call_with_current(this, action, er, pos);
//	}

//	void cEventReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
//	{
//		auto& drops = ((cEventReceiverPrivate*)this)->acceptable_drops;
//		drops.resize(drop_count);
//		for (auto i = 0; i < drop_count; i++)
//			drops[i] = _drops[i];
//	}

	void* cEventReceiverPrivate::add_key_listener(bool (*callback)(Capture& c, KeyStateFlags action, uint value), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		key_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_key_listener(void* lis)
	{
		std::erase_if(key_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
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

	void cEventReceiverPrivate::on_gain_dispatcher()
	{
		dispatcher->dirty = true;
	}

	void cEventReceiverPrivate::on_lost_dispatcher()
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

		((EntityPrivate*)entity)->set_state((StateFlags)((int)((EntityPrivate*)entity)->state & (~StateHovering) & (~StateFocusing) & (~StateActive)));

		dispatcher->dirty = true;
	}

	void cEventReceiverPrivate::on_entity_visibility_changed()
	{
		if (dispatcher)
			dispatcher->dirty = true;
	}

	cEventReceiver* cEventReceiver::create()
	{
		return f_new<cEventReceiverPrivate>();
	}
}
