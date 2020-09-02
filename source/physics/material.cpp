#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		MaterialPrivate::MaterialPrivate(DevicePrivate* d, float static_friction, float dynamic_friction, float restitution)
		{
#ifdef USE_PHYSX
			px_material = d->px_instance->createMaterial(static_friction, dynamic_friction, restitution);
#endif
		}

		MaterialPrivate::~MaterialPrivate()
		{
#ifdef USE_PHYSX
			px_material->release();
#endif
		}

		Material* Material::create(Device *d, float static_friction, float dynamic_friction, float restitution)
		{
			return new MaterialPrivate((DevicePrivate*)d, static_friction, dynamic_friction, restitution);
		}
	}
}

