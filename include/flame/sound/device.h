#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Device
		{
			virtual void release() = 0;

			FLAME_SOUND_EXPORTS static Device* get();
			FLAME_SOUND_EXPORTS static Device* create(bool as_default = true);
		};

		struct Recorder
		{
			virtual void release() = 0;

			virtual void start_record() = 0;
			virtual void stop_record(void* dst) = 0;

			FLAME_SOUND_EXPORTS static Recorder* get();
			FLAME_SOUND_EXPORTS static Recorder* create(uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f /* define buffer size for duration second */, bool as_default = true);
		};
	}
}
