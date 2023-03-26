#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cReceiver : Component
	{
		struct Create
		{
			virtual cReceiverPtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
