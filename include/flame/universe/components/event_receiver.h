#pragma once

#include <flame/universe/component.h>

namespace flame
{
//	struct sEventDispatcher;
//	struct cElement;
//
	struct FLAME_RU(cEventReceiver : Component, all)
	{
		inline static auto type_name = "cEventReceiver";
		inline static auto type_hash = ch(type_name);

		cEventReceiver() :
			Component(type_name, type_hash, true)
		{
		}

//		uint drag_hash; // non-zero means it can be draged to drop
//
//		ListenerHub<bool(Capture& c, cEventReceiver* er, bool* pass)>								pass_checkers;
//		ListenerHub<bool(Capture& c, KeyStateFlags action, int value)>								key_listeners;
//		ListenerHub<bool(Capture& c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos)>		drag_and_drop_listeners;
//		ListenerHub<bool(Capture& c, bool hovering)>												hover_listeners;
//		ListenerHub<bool(Capture& c, bool focusing)>												focus_listeners;
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
//		FLAME_UNIVERSE_EXPORTS void on_key(KeyStateFlags action, uint value);
//		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value);
//		FLAME_UNIVERSE_EXPORTS void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);

		virtual void* add_mouse_listener(bool (*callback)(Capture& c, KeyStateFlags action, MouseKey key, const Vec2i& pos), const Capture& capture) = 0;
		virtual void remove_mouse_listener(void* lis) = 0;

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* create();
	};
}
