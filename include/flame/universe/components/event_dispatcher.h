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

		cEventReceiver* hovering;
		cEventReceiver* focusing;

		cEventDispatcher() :
			Component("EventDispatcher")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cEventDispatcher() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cEventDispatcher* create(Window* window = nullptr);
	};
}
