#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct Window;
	struct cEventReceiver;

	struct sEventDispatcher : System
	{
		inline static auto type_name = "sEventDispatcher";
		inline static auto type_hash = ch(type_name);

		sEventDispatcher() :
			System(type_name, type_hash)
		{
		}

//		KeyStateFlags key_states[Key_Count];
//		KeyStateFlags mouse_buttons[3];
//		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
//		int mouse_scroll;
//
//		cEventReceiver* hovering;
//		cEventReceiver* focusing;
//		FocusingState focusing_state;
//		cEventReceiver* key_receiving;
//		cEventReceiver* drag_overing;
//
//		cEventReceiver* next_focusing;
//
//		bool pending_update;
//
		FLAME_UNIVERSE_EXPORTS static sEventDispatcher* create();
	};
}
