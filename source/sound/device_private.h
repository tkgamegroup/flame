#pragma once

#include "device.h"
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		extern DevicePtr current_device;

		struct DevicePrivate : Device
		{
			ALCdevice *al_dev;
			ALCcontext* al_ctx;

			~DevicePrivate();
		};

		struct RecorderPrivate : Recorder
		{
			ALCdevice* al_dev;

			~RecorderPrivate();

			void start_record() override;
			void stop_record(void* dst) override;
		};
	}
}
