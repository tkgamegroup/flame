#pragma once

#include "device.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate : Device
		{
			PxFoundation* px_foundation;
			PxPvd* px_pvd;
			PxPhysics* px_instance;
			PxCooking* px_cooking;
			PxDefaultAllocator px_allocator;
			PxDefaultErrorCallback px_error_callback;

			std::vector<std::unique_ptr<MaterialPrivate>> materials;

			DevicePrivate();
			~DevicePrivate();

			void release() override { delete this; }
		};

		extern DevicePrivate* current_device;
	}
}

