#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		thread_local DevicePrivate* default_device = nullptr;

		DevicePrivate::DevicePrivate()
		{
#ifdef USE_PHYSX
			px_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, px_allocator, px_error_callback);
			{
				px_pvd = PxCreatePvd(*px_foundation);
				auto transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
				px_pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
			}
			px_instance = PxCreatePhysics(PX_PHYSICS_VERSION, *px_foundation, PxTolerancesScale(), true, px_pvd);
			{
				PxTolerancesScale scale;
				px_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *px_foundation, PxCookingParams(scale));
			}
#endif
			mat.reset(new MaterialPrivate(this, 0.2f, 0.2f, 0.3f));
		}

		DevicePrivate::~DevicePrivate()
		{
			if (default_device == this)
				default_device = nullptr;
		}

		Device* Device::get_default()
		{
			return default_device;
		}

		void Device::set_default(Device* device)
		{
			default_device = (DevicePrivate*)device;
		}

		Device* Device::create()
		{
			return new DevicePrivate;
		}
	}
}

