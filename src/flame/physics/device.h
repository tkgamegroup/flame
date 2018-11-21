#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate;

		struct Device
		{
			DevicePrivate *_priv;
		};

		FLAME_PHYSICS_EXPORTS Device *create_device();
		FLAME_PHYSICS_EXPORTS void destroy_device(Device *d);
	}
}

