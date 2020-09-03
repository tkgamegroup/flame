#pragma once

#include <flame/physics/material.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct MaterialPrivate : Material
		{
#ifdef USE_PHYSX
			PxMaterial* px_material;
#endif

			MaterialPrivate(float static_friction, float dynamic_friction, float restitution);
			~MaterialPrivate();

			void release() override { delete this; }
		};
	}
}

