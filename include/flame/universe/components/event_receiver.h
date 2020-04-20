#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct sEventDispatcher;
	struct cElement;

	struct FLAME_R(cEventReceiver) : Component
	{
		sEventDispatcher* dispatcher;
		cElement* element;

		FocusType focus_type;
		uint drag_hash; // non-zero means it can be draged to drop
		FLAME_RV(EventReceiverState, state, m);

		ListenerHub<bool(void* c, cEventReceiver* er, bool* pass)>								pass_checkers;
		ListenerHub<bool(void* c, KeyStateFlags action, int value)>								key_listeners;
		ListenerHub<bool(void* c, KeyStateFlags action, MouseKey key, const Vec2i& pos)>		mouse_listeners;
		ListenerHub<bool(void* c, DragAndDrop action, cEventReceiver* er, const Vec2i& pos)>	drag_and_drop_listeners;
		ListenerHub<bool(void* c, bool hovering)>												hover_listeners;
		ListenerHub<bool(void* c, bool focusing)>												focus_listeners;
		ListenerHub<bool(void* c, EventReceiverState state)>									state_listeners;

		cEventReceiver() :
			Component("cEventReceiver")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_acceptable_drops(uint drop_count, const uint* drops);

		FLAME_UNIVERSE_EXPORTS void on_key(KeyStateFlags action, uint value);
		FLAME_UNIVERSE_EXPORTS void on_mouse(KeyStateFlags action, MouseKey key, const Vec2i& value);
		FLAME_UNIVERSE_EXPORTS void on_drag_and_drop(DragAndDrop action, cEventReceiver* er, const Vec2i& pos);

		FLAME_UNIVERSE_EXPORTS static cEventReceiver* current();
		FLAME_UNIVERSE_EXPORTS static void set_linked_object(cEventReceiver* e);
		FLAME_UNIVERSE_EXPORTS static cEventReceiver* FLAME_RF(get_linked_object)();
		FLAME_UNIVERSE_EXPORTS static cEventReceiver* FLAME_RF(create)();
	};
}
