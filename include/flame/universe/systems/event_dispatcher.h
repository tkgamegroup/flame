#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cEventReceiver;

	struct sEventDispatcher : System
	{
		KeyStateFlags key_states[KeyCount];
		KeyStateFlags mouse_buttons[3];
		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;

		cEventReceiver* hovering;
		cEventReceiver* focusing;
		FocusingState focusing_state;
		cEventReceiver* key_receiving;
		cEventReceiver* drag_overing;

		cEventReceiver* next_focusing;

		bool pending_update;

		sEventDispatcher() :
			System("sEventDispatcher")
		{
		}

		FLAME_UNIVERSE_EXPORTS cEventReceiver* get_next_mouse_hovering();
		FLAME_UNIVERSE_EXPORTS void set_event_passthrough();

		FLAME_UNIVERSE_EXPORTS static sEventDispatcher* create();
	};
}
