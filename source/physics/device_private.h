#pragma once

#include <flame/physics/device.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate : Device
		{
#ifdef USE_PHYSX
			PxDefaultAllocator allocator;
			PxDefaultErrorCallback error_callback;
			PxFoundation* foundation;
			PxPhysics* inst;
#endif

			DevicePrivate();

			void release() override { delete this; }
		};
	}
}

