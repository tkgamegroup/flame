#include "device_private.h"

ALCdevice* al_device = nullptr;
ALCcontext* al_content = nullptr;

namespace flame
{
	namespace audio
	{
		struct DeviceCreate : Device::Create
		{
			DevicePtr operator()() override
			{
				auto ret = new DevicePrivate;
				al_device = alcOpenDevice(nullptr);
				al_content = alcCreateContext(al_device, nullptr);
				alcMakeContextCurrent(al_content);
				auto al_err = alGetError();
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
