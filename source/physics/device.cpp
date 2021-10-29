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
				px_pvd->connect(*PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10), 
					physx::PxPvdInstrumentationFlag::eALL);
			}
			px_instance = PxCreatePhysics(PX_PHYSICS_VERSION, *px_foundation, PxTolerancesScale(), true, px_pvd);
			{
				PxTolerancesScale scale;
				px_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *px_foundation, PxCookingParams(scale));
			}
#endif
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
			auto ret = new DevicePrivate;
			if (!default_device)
				default_device = ret;
			return ret;
		}
	}
}

