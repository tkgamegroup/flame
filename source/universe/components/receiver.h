#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cReceiver : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		Listeners<void()> click_listeners;

		struct Create
		{
			virtual cReceiverPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
