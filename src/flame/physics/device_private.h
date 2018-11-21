#pragma once

#include "device.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate
		{
			PxDefaultAllocator allocator;
			PxDefaultErrorCallback error_callback;
			PxFoundation *foundation;
			PxPhysics *inst;
		};
	}
}

