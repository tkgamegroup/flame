#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct Window;

	struct cEventReceiver;

	struct sEventDispatcher : System
	{
		uint key_states[KeyCount];
		uint mouse_buttons[3];
		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;

		cEventReceiver* hovering;
		cEventReceiver* focusing;
		cEventReceiver* drag_overing;

		cEventReceiver* next_focusing;

		bool pending_update;

		sEventDispatcher() :
			System("EventDispatcher")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sEventDispatcher* create(Window* window);
	};
}
