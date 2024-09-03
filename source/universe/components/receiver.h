#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cReceiver : Component
	{
		enum Type
		{
			TypeRect,
			TypeCircle
		};

		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		Type type = TypeRect;

		Listeners<void(uint type, const vec2& value)> event_listeners;

		// Reflect
		VirtualUdt<Action>	click_action;
		// Reflect
		VirtualUdt<Action>	mouse_enter_action;
		// Reflect
		VirtualUdt<Action>	mouse_leave_action;

		struct Create
		{
			virtual cReceiverPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
