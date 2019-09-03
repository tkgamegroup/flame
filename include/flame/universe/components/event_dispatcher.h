#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct Window;

	struct cEventReceiver;

	struct cEventDispatcher : Component
	{
		uint key_states[KeyCount];
		uint mouse_buttons[3];
		Vec2i mouse_pos, mouse_pos_prev, mouse_disp;
		int mouse_scroll;

		cEventReceiver* hovering;
		cEventReceiver* focusing;
		cEventReceiver* drag_overing;

		cEventDispatcher() :
			Component("EventDispatcher")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cEventDispatcher() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cEventDispatcher* create(Window* window = nullptr);
	};
}
