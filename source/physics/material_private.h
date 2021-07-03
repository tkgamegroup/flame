#pragma once

#include "material.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct MaterialPrivate : Material
		{
			float static_friction;
			float dynamic_friction;
			float restitution;

#ifdef USE_PHYSX
			UniPtr<PxMaterial> px_material;
#endif

			MaterialPrivate(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution);

			void release() override { delete this; }

			static MaterialPrivate* get(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution);
		};
	}
}

