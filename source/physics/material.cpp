#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		MaterialPrivate::MaterialPrivate(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution) :
			static_friction(static_friction),
			dynamic_friction(dynamic_friction),
			restitution(restitution)
		{
#ifdef USE_PHYSX
			px_material.reset(device->px_instance->createMaterial(static_friction, dynamic_friction, restitution));
#endif
		}

		MaterialPrivate* MaterialPrivate::get(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution)
		{
			for (auto& m : device->materials)
			{
				if (epsilonEqual(m->static_friction, static_friction, 0.0001f) &&
					epsilonEqual(m->dynamic_friction, dynamic_friction, 0.0001f) &&
					epsilonEqual(m->restitution, restitution, 0.0001f))
					return m.get();
			}
			auto m = new MaterialPrivate(device, static_friction, dynamic_friction, restitution);
			device->materials.emplace_back(m);
			return m;
		}

		Material* Material::get(Device* device, float static_friction, float dynamic_friction, float restitution)
		{
			return MaterialPrivate::get((DevicePrivate*)device, static_friction, dynamic_friction, restitution);
		}
	}
}

