#pragma once

#include "../component.h"

namespace flame
{
	struct cReceiver : Component
	{
		inline static auto type_name = "flame::cReceiver";
		inline static auto type_hash = ch(type_name);

		cReceiver() :
			Component(type_name, type_hash)
		{
		}

		virtual bool get_mute() const = 0;
		virtual void set_mute(bool v) = 0;

		virtual bool get_floating() const = 0;
		virtual void set_floating(bool v) = 0;

//		uint drag_hash; // non-zero means it can be draged to drop

//		ListenerHub<bool(Capture& c, DragAndDrop action, cReceiver* er, const ivec2& pos)>		drag_and_drop_listeners;

//		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(uint drops_count, const uint* drops);

		virtual void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_down_listener(void* lis) = 0;
		virtual void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_up_listener(void* lis) = 0;
		virtual void* add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture) = 0;
		virtual void remove_char_listener(void* lis) = 0;
		virtual void* add_mouse_left_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_down_listener(void* lis) = 0;
		virtual void* add_mouse_left_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_up_listener(void* lis) = 0;
		virtual void* add_mouse_right_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_down_listener(void* lis) = 0;
		virtual void* add_mouse_right_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_up_listener(void* lis) = 0;
		virtual void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_down_listener(void* lis) = 0;
		virtual void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_up_listener(void* lis) = 0;
		virtual void* add_mouse_move_listener(void (*callback)(Capture& c, const ivec2& disp, const ivec2& pos), const Capture& capture) = 0;
		virtual void remove_mouse_move_listener(void* lis) = 0;
		virtual void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) = 0;
		virtual void remove_mouse_scroll_listener(void* lis) = 0;
		virtual void* add_mouse_click_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_mouse_click_listener(void* lis) = 0;
		virtual void* add_mouse_dbclick_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_mouse_dbclick_listener(void* lis) = 0;

		virtual void on_key_event(KeyboardKey key, bool down = true) = 0;

		virtual const char* get_tooltip() const = 0;
		virtual void set_tooltip(const char* str) = 0;

		FLAME_UNIVERSE_EXPORTS static cReceiver* create();
	};
}
