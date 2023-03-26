#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cImage : Component
	{

		struct Create
		{
			virtual cImagePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
