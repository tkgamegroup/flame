#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		MaterialPrivate::MaterialPrivate(float static_friction, float dynamic_friction, float restitution)
		{
#ifdef USE_PHYSX
			px_material = DevicePrivate::get()->px_instance->createMaterial(static_friction, dynamic_friction, restitution);
#endif
		}

		MaterialPrivate::~MaterialPrivate()
		{
#ifdef USE_PHYSX
			px_material->release();
#endif
		}

		Material* Material::create(float static_friction, float dynamic_friction, float restitution)
		{
			return new MaterialPrivate(static_friction, dynamic_friction, restitution);
		}
	}
}

