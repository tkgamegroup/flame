#include "device_private.h"

namespace flame
{
	namespace physics
	{
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
		}

		static DevicePrivate* device = nullptr;

		DevicePrivate* DevicePrivate::get()
		{
			if (!device)
				device = new DevicePrivate;
			return device;
		}

		Device* Device::get()
		{
			return DevicePrivate::get();
		}
	}
}

