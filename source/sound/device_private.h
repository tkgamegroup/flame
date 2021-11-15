#pragma once

#include "device.h"
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct DevicePrivate : Device
		{
			ALCdevice *al_dev;
			ALCcontext* al_ctx;

			DevicePrivate();
			~DevicePrivate();
		};

		extern DevicePrivate* default_device;

		struct RecorderPrivate : Recorder
		{
			ALCdevice* al_dev;

			RecorderPrivate(uint frequency, bool stereo, bool _16bit, float duration);
			~RecorderPrivate();

			void start_record() override;
			void stop_record(void* dst) override;
		};
	}
}
