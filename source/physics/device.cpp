#include "device_private.h"

namespace flame
{
	namespace physics
	{
		DevicePrivate::DevicePrivate()
		{
#ifdef USE_PHYSX
			foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, allocator, error_callback);
			inst = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());
#endif
		}

		Device* Device::create()
		{
			return new DevicePrivate;
		}
	}
}

