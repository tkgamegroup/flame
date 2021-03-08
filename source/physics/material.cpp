#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		MaterialPrivate::MaterialPrivate(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution)
		{
#ifdef USE_PHYSX
			px_material.reset(device->px_instance->createMaterial(static_friction, dynamic_friction, restitution));
#endif
		}

		Material* Material::create(Device* device, float static_friction, float dynamic_friction, float restitution)
		{
			return new MaterialPrivate((DevicePrivate*)device, static_friction, dynamic_friction, restitution);
		}
	}
}

