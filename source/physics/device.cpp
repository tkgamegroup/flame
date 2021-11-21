#include "device_private.h"
#include "material_private.h"

namespace flame
{
	namespace physics
	{
		DevicePrivate* current_device = nullptr;

		DevicePrivate::DevicePrivate()
		{
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
		}

		DevicePrivate::~DevicePrivate()
		{
			if (current_device == this)
				current_device = nullptr;
		}

		Device* Device::get_default()
		{
			return current_device;
		}

		void Device::set_default(Device* device)
		{
			current_device = (DevicePrivate*)device;
		}

		Device* Device::create()
		{
			auto ret = new DevicePrivate;
			if (!current_device)
				current_device = ret;
			return ret;
		}
	}
}

