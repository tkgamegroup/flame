#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;

		struct Material
		{
			virtual void release() = 0;

			FLAME_PHYSICS_EXPORTS static Material* create(Device* d, float static_friction, float dynamic_friction, float restitution);
		};
	}
}

