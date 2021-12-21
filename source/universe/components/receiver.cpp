#include "../world_private.h"
#include "element_private.h"
#include "receiver_private.h"
#include "../systems/dispatcher_private.h"

namespace flame
{
	bool cReceiverPrivate::is_active()
	{
		return dispatcher->active == this;
	}

	void* cReceiverPrivate::add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) 
	{
		return key_down_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_key_down_listener(void* lis)
	{
		key_down_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture)
	{
		return key_up_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_key_up_listener(void* lis)
	{
		key_up_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture)
	{
		return char_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_char_listener(void* lis)
	{
		char_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_left_down_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_left_down_listener(void* lis)
	{
		mouse_left_down_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_left_up_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_left_up_listener(void* lis)
	{
		mouse_left_up_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_right_down_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_right_down_listener(void* lis)
	{
		mouse_right_down_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_right_up_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_right_up_listener(void* lis)
	{
		mouse_right_up_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_middle_down_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_middle_down_listener(void* lis)
	{
		mouse_middle_down_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		return mouse_middle_up_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_middle_up_listener(void* lis)
	{
		mouse_middle_up_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& disp, const ivec2& pos), const Capture& capture)
	{
		return mouse_move_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_move_listener(void* lis)
	{
		mouse_move_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture)
	{
		return mouse_scroll_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_scroll_listener(void* lis)
	{
		mouse_scroll_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_click_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		return mouse_click_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_click_listener(void* lis)
	{
		mouse_click_listeners.remove(lis);
	}

	void* cReceiverPrivate::add_mouse_dbclick_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		return mouse_dbclick_listeners.add(callback, capture);
	}

	void cReceiverPrivate::remove_mouse_dbclick_listener(void* lis)
	{
		mouse_dbclick_listeners.remove(lis);
	}

	void cReceiverPrivate::on_key_event(KeyboardKey key, bool down)
	{
		if (down)
		{
			for (auto& l : key_down_listeners.list)
				l->call(key);
		}
		else
		{
			for (auto& l : key_up_listeners.list)
				l->call(key);
		}
	}

	void cReceiverPrivate::on_added()
	{
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);
	}

	void cReceiverPrivate::on_visibility_changed(bool v)
	{
		if (dispatcher)
			dispatcher->dirty = true;
	}

	void cReceiverPrivate::on_entered_world()
	{
		dispatcher = entity->world->get_system_t<sDispatcherPrivate>();
		assert(dispatcher);
		dispatcher->dirty = true;
	}

//	void cReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
//	{
//		auto& drops = ((cReceiverPrivate*)this)->acceptable_drops;
//		drops.resize(drop_count);
//		for (auto i = 0; i < drop_count; i++)
//			drops[i] = _drops[i];
//	}

	void cReceiverPrivate::on_left_world()
	{
		if (this == dispatcher->hovering)
			dispatcher->hovering = nullptr;
		if (this == dispatcher->focusing)
		{
			dispatcher->focusing = nullptr;
			dispatcher->active = nullptr;
			dispatcher->dragging = nullptr;
		}
		if (this == dispatcher->keyboard_target)
			dispatcher->keyboard_target = nullptr;
		if (this == dispatcher->drag_overing)
			dispatcher->drag_overing = nullptr;
		//if (er == next_focusing)
		//	next_focusing = (cReceiver*)INVALID_POINTER;

		entity->set_state((StateFlags)((int)entity->state & (~StateHovering) & (~StateFocusing) & (~StateActive)));

		dispatcher->dirty = true;
	}

	cReceiver* cReceiver::create()
	{
		return new cReceiverPrivate();
	}
}
