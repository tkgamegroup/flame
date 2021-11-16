#pragma once

#include "sound.h"

namespace flame
{
	namespace sound
	{
		struct Device
		{
			virtual ~Device() {}

			FLAME_SOUND_EXPORTS static DevicePtr create();
		};

		FLAME_SOUND_EXPORTS extern DevicePtr default_device;

		struct Recorder
		{
			virtual ~Recorder() {}

			virtual void start_record() = 0;
			virtual void stop_record(void* dst) = 0;

			FLAME_SOUND_EXPORTS static RecorderPtr create(uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f /* define buffer size for duration second */);
		};
	}
}
