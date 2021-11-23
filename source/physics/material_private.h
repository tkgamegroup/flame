#pragma once

#include "material.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct MaterialPrivate : Material
		{
			DevicePrivate* device;

			float static_friction;
			float dynamic_friction;
			float restitution;

			std::unique_ptr<PxMaterial> px_material;

			MaterialPrivate(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution);

			void release() override { delete this; }

			static MaterialPrivate* get(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution);
		};
	}
}

