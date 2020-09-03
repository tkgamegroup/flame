#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static Device* get();
		};
	}
}

