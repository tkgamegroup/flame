#include "device_private.h"

namespace flame
{
	namespace audio
	{
		struct DeviceCreate : Device::Create
		{
			DevicePtr operator()() override
			{

			}
		}Device_create;
		Device::Create& Device::create = Device_create;

		struct DeviceCurrent : Device::Current
		{
			DevicePtr& operator()() override
			{

			}
		}Device_current;
		Device::Current& Device::current = Device_current;
	}
}
