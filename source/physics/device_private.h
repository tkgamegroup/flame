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
			physx::PxFoundation* px_foundation;
			physx::PxPhysics* px_instance;
			physx::PxDefaultAllocator px_allocator;
			physx::PxDefaultErrorCallback px_error_callback;
#endif

			DevicePrivate();

			void release() override { delete this; }

			static DevicePrivate* get();
		};
	}
}

