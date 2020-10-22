#include <flame/script/script.h>
#include "../world_private.h"
#include "element_private.h"
#include "event_receiver_private.h"
#include "../systems/event_dispatcher_private.h"

namespace flame
{
	cEventReceiverPrivate::~cEventReceiverPrivate()
	{
		for (auto s : key_down_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : key_up_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : mouse_left_down_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : mouse_left_up_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : mouse_move_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : mouse_scroll_listeners_s)
			script::Instance::get_default()->release_slot(s);
		for (auto s : mouse_click_listeners_s)
			script::Instance::get_default()->release_slot(s);
	}

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

	void* cEventReceiverPrivate::add_mouse_left_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_left_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_right_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_right_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_middle_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_middle_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture)
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

	void* cEventReceiverPrivate::add_mouse_move_listener(void (*callback)(Capture& c, const Vec2i& disp, const Vec2i& pos), const Capture& capture)
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

	void cEventReceiverPrivate::add_key_down_listener_s(uint slot)
	{
		key_down_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_key_down_listener_s(uint slot)
	{
		for (auto it = key_down_listeners_s.begin(); it != key_down_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				key_down_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_key_up_listener_s(uint slot)
	{
		key_up_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_key_up_listener_s(uint slot)
	{
		for (auto it = key_up_listeners_s.begin(); it != key_up_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				key_up_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_mouse_left_down_listener_s(uint slot)
	{
		mouse_left_down_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_mouse_left_down_listener_s(uint slot)
	{
		for (auto it = mouse_left_down_listeners_s.begin(); it != mouse_left_down_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				mouse_left_down_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_mouse_left_up_listener_s(uint slot)
	{
		mouse_left_up_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_mouse_left_up_listener_s(uint slot)
	{
		for (auto it = mouse_left_up_listeners_s.begin(); it != mouse_left_up_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				mouse_left_up_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_mouse_move_listener_s(uint slot)
	{
		mouse_move_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_mouse_move_listener_s(uint slot)
	{
		for (auto it = mouse_move_listeners_s.begin(); it != mouse_move_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				mouse_move_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_mouse_scroll_listener_s(uint slot)
	{
		mouse_scroll_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_mouse_scroll_listener_s(uint slot)
	{
		for (auto it = mouse_scroll_listeners_s.begin(); it != mouse_scroll_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				mouse_scroll_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::add_mouse_click_listener_s(uint slot)
	{
		mouse_click_listeners_s.push_back(slot);
	}

	void cEventReceiverPrivate::remove_mouse_click_listener_s(uint slot)
	{
		for (auto it = mouse_click_listeners_s.begin(); it != mouse_click_listeners_s.end(); it++)
		{
			if (*it == slot)
			{
				mouse_click_listeners_s.erase(it);
				script::Instance::get_default()->release_slot(slot);
			}
		}
	}

	void cEventReceiverPrivate::on_key_event(KeyboardKey key, bool down)
	{
		auto& listeners = down ? key_down_listeners : key_up_listeners;
		auto& listeners_s = down ? key_down_listeners_s : key_up_listeners_s;
		for (auto& l : listeners)
			l->call(key);
		{
			script::Parameter p;
			p.type = script::ScriptTypeInt;
			p.data.i[0] = key;
			for (auto s : listeners_s)
				script::Instance::get_default()->call_slot(s, 1, &p);
		}
	}

//	void cEventReceiver::set_acceptable_drops(uint drop_count, const uint* _drops)
//	{
//		auto& drops = ((cEventReceiverPrivate*)this)->acceptable_drops;
//		drops.resize(drop_count);
//		for (auto i = 0; i < drop_count; i++)
//			drops[i] = _drops[i];
//	}

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
		if (this == dispatcher->keyboard_target)
			dispatcher->keyboard_target = nullptr;
		if (this == dispatcher->drag_overing)
			dispatcher->drag_overing = nullptr;
		//if (er == next_focusing)
		//	next_focusing = (cEventReceiver*)INVALID_POINTER;

		entity->set_state((StateFlags)((int)entity->state & (~StateHovering) & (~StateFocusing) & (~StateActive)));

		dispatcher->dirty = true;
	}

	void cEventReceiverPrivate::on_local_message(Message msg, void* p)
	{
		switch (msg)
		{
		case MessageVisibilityChanged:
			if (dispatcher)
				dispatcher->dirty = true;
			break;
		}
	}

	cEventReceiver* cEventReceiver::create()
	{
		return f_new<cEventReceiverPrivate>();
	}
}
