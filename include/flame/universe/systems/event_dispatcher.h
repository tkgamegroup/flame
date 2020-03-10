#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cEventReceiver;

	struct MouseEventDispatcher
	{

	};

	struct sEventDispatcher : System
	{
		KeyStateFlags key_states[KeyCount];
		KeyStateFlags mouse_buttons[3];
		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;

		cEventReceiver* hovering;
		cEventReceiver* focusing;
		FocusingState focusing_state;
		cEventReceiver* drag_overing;

		cEventReceiver* next_focusing;

		bool pending_update;

		sEventDispatcher() :
			System("sEventDispatcher")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sEventDispatcher* create();
	};
}
