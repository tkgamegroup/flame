#pragma once

#include <flame/sound/device.h>
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct DevicePrivate : Device
		{
			ALCdevice *al_dev;

			DevicePrivate();
			DevicePrivate(uint frequency, bool stereo, bool _16bit, float duration);
			~DevicePrivate();

			void start_record();
			void stop_record(void* dst);
		};
	}
}
