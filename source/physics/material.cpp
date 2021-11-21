#include "material_private.h"
#include "device_private.h"

namespace flame
{
	namespace physics
	{
		MaterialPrivate::MaterialPrivate(DevicePrivate* _device, float static_friction, float dynamic_friction, float restitution) :
			device(_device),
			static_friction(static_friction),
			dynamic_friction(dynamic_friction),
			restitution(restitution)
		{
			if (!device)
				device = current_device;

			px_material.reset(device->px_instance->createMaterial(static_friction, dynamic_friction, restitution));
		}

		MaterialPrivate* MaterialPrivate::get(DevicePrivate* device, float static_friction, float dynamic_friction, float restitution)
		{
			if (!device)
				device = current_device;
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

