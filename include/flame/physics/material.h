#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;

		struct MaterialPrivate;

		struct Material
		{
			MaterialPrivate *_priv;
		};

		FLAME_PHYSICS_EXPORTS Material *create_material(Device *d, float static_friction, float dynamic_friction, float restitution);
		FLAME_PHYSICS_EXPORTS void destroy_material(Material *m);
	}
}

