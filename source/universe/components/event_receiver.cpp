#include <flame/script/script.h>
#include "../world_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	void cEventReceiverPrivate::set_ignore_occluders(bool v)
	{
		ignore_occluders = v;
	}

	void* cEventReceiverPrivate::add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) 
	{
		auto c = new Closure(callback, capture);
		key_down_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_key_down_listener(void* lis)
	{
		std::erase_if(key_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		key_up_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_key_up_listener(void* lis)
	{
		std::erase_if(key_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		char_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_char_listener(void* lis)
	{
		std::erase_if(char_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_left_down_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_left_down_listener(void* lis)
	{
		std::erase_if(mouse_left_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_left_up_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_left_up_listener(void* lis)
	{
		std::erase_if(mouse_left_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_right_down_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_right_down_listener(void* lis)
	{
		std::erase_if(mouse_right_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_right_up_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_right_up_listener(void* lis)
	{
		std::erase_if(mouse_right_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_middle_down_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_middle_down_listener(void* lis)
	{
		std::erase_if(mouse_middle_down_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_middle_up_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_middle_up_listener(void* lis)
	{
		std::erase_if(mouse_middle_up_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& disp, const ivec2& pos), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_move_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_move_listener(void* lis)
	{
		std::erase_if(mouse_move_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_scroll_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_scroll_listener(void* lis)
	{
		std::erase_if(mouse_scroll_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_click_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_click_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_click_listener(void* lis)
	{
		std::erase_if(mouse_click_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void* cEventReceiverPrivate::add_mouse_dbclick_listener(void (*callback)(Capture& c), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		mouse_dbclick_listeners.emplace_back(c);
		return c;
	}

	void cEventReceiverPrivate::remove_mouse_dbclick_listener(void* lis)
	{
		std::erase_if(mouse_dbclick_listeners, [&](const auto& i) {
			return i == (decltype(i))lis;
		});
	}

	void cEventReceiverPrivate::on_key_event(KeyboardKey key, bool down)
	{
		auto& listeners = down ? key_down_listeners : key_up_listeners;
		for (auto& l : listeners)
			l->call(key);
	}

	void cEventReceiverPrivate::on_added()
	{
		element = entity->get_component_t<cElementPrivate>();
		fassert(element);
	}

	void cEventReceiverPrivate::on_visibility_changed(bool v)
	{
		if (dispatcher)
			dispatcher->dirty = true;
	}

	void cEventReceiverPrivate::on_entered_world()
	{
		dispatcher = entity->world->get_system_t<sEventDispatcherPrivate>();
		fassert(dispatcher);
		dispatcher->dirty = true;
	}

//	void cEventReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
//	{
//		auto& drops = ((cEventReceiverPrivate*)this)->acceptable_drops;
//		drops.resize(drop_count);
//		for (auto i = 0; i < drop_count; i++)
//			drops[i] = _drops[i];
//	}

	void cEventReceiverPrivate::on_left_world()
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
		//	next_focusing = (cEventReceiver*)INVALID_POINTER;

		entity->set_state((StateFlags)((int)entity->state & (~StateHovering) & (~StateFocusing) & (~StateActive)));

		dispatcher->dirty = true;
	}

	cEventReceiver* cEventReceiver::create()
	{
		return f_new<cEventReceiverPrivate>();
	}
}
