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
			PxFoundation* px_foundation;
			PxPvd* px_pvd;
			PxPhysics* px_instance;
			PxCooking* px_cooking;
			PxDefaultAllocator px_allocator;
			PxDefaultErrorCallback px_error_callback;
#endif

			DevicePrivate();

			void release() override { delete this; }

			static DevicePrivate* get();
		};
	}
}

