#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cEventReceiver : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cEventReceiver";
		inline static auto type_hash = ch(type_name);

		cEventReceiver() :
			Component(type_name, type_hash)
		{
		}

//		uint drag_hash; // non-zero means it can be draged to drop
//
//		ListenerHub<bool(Capture& c, cEventReceiver* er, bool* pass)>								pass_checkers;
//		ListenerHub<bool(Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos)>		drag_and_drop_listeners;
//
//		FLAME_RV(ListenerHub<void(Capture& c)>,														clicked_listeners);
//
//		bool is_hovering()
//		{
//			return dispatcher->hovering == this;
//		}
//
//		bool is_focusing()
//		{
//			return dispatcher->focusing == this;
//		}
//
//		bool is_focusing_and_not_normal()
//		{
//			return dispatcher->focusing == this && dispatcher->focusing_state != FocusingNormal;
//		}
//
//		bool is_active()
//		{
//			return dispatcher->focusing == this && dispatcher->focusing_state == FocusingAndActive;
//		}
//
//		bool is_dragging()
//		{
//			return dispatcher->focusing == this && dispatcher->focusing_state == FocusingAndDragging;
//		}
//
//		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(uint drops_count, const uint* drops);
//
//		FLAME_UNIVERSE_EXPORTS void send_key_event(KeyStateFlags action, uint value);
//		FLAME_UNIVERSE_EXPORTS void send_mouse_event(KeyStateFlags action, MouseKey key, const Vec2i& value);
//		FLAME_UNIVERSE_EXPORTS void send_drag_and_drop_event(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);

		virtual void* add_key_down_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_down_listener(void* lis) = 0;
		virtual void* add_key_up_listener(void (*callback)(Capture& c, KeyboardKey key), const Capture& capture) = 0;
		virtual void remove_key_up_listener(void* lis) = 0;
		virtual void* add_char_listener(void (*callback)(Capture& c, wchar_t ch), const Capture& capture) = 0;
		virtual void remove_char_listener(void* lis) = 0;
		virtual void* add_mouse_left_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_down_listener(void* lis) = 0;
		virtual void* add_mouse_left_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_left_up_listener(void* lis) = 0;
		virtual void* add_mouse_right_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_down_listener(void* lis) = 0;
		virtual void* add_mouse_right_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_right_up_listener(void* lis) = 0;
		virtual void* add_mouse_middle_down_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_down_listener(void* lis) = 0;
		virtual void* add_mouse_middle_up_listener(void (*callback)(Capture& c, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_middle_up_listener(void* lis) = 0;
		virtual void* add_mouse_move_listener(void (*callback)(Capture& c, const Vec2i& disp, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_move_listener(void* lis) = 0;
		virtual void* add_mouse_scroll_listener(void (*callback)(Capture& c, int scroll), const Capture& capture) = 0;
		virtual void remove_mouse_scroll_listener(void* lis) = 0;
		virtual void* add_mouse_click_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_mouse_click_listener(void* lis) = 0;
		virtual void* add_mouse_dbclick_listener(void (*callback)(Capture& c), const Capture& capture) = 0;
		virtual void remove_mouse_dbclick_listener(void* lis) = 0;

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
