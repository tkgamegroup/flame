#pragma once

#include <flame/sound/device.h>
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct DevicePrivate : Device
		{
			DeviceType type;

			ALCdevice *al_dev;

			DevicePrivate(DeviceType t);
			~DevicePrivate();

			void start_record();
			int get_recorded_samples();
			void get_recorded_data(void* dst, int samples);
			void stop_record();
		};
	}
}
