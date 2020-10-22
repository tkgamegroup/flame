#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static Device* get_default();
			FLAME_PHYSICS_EXPORTS static void set_default(Device* device);
			FLAME_PHYSICS_EXPORTS static Device* create();
		};
	}
}

