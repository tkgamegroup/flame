#include "device_private.h"

namespace flame
{
	namespace physics
	{
		DevicePrivate::DevicePrivate()
		{
#ifdef USE_PHYSX
			px_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, px_allocator, px_error_callback);
			px_instance = PxCreatePhysics(PX_PHYSICS_VERSION, *px_foundation, physx::PxTolerancesScale());
#endif
		}

		Device* Device::create()
		{
			return new DevicePrivate;
		}
	}
}

