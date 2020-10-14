#pragma once

#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Scene;

		struct Controller
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static Controller* create();
		};
	}
}
