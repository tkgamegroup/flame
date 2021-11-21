#pragma once

#include "sound.h"

namespace flame
{
	namespace sound
	{
		struct Device
		{
			virtual ~Device() {}

			struct Create
			{
				virtual DevicePtr operator()() = 0;
			};
			FLAME_SOUND_EXPORTS static Create& create;

			struct Current
			{
				virtual DevicePtr& operator()() = 0;
			};
			FLAME_SOUND_EXPORTS static Current& current;
		};

		struct Recorder
		{
			virtual ~Recorder() {}

			virtual void start_record() = 0;
			virtual void stop_record(void* dst) = 0;

			struct Create
			{
				virtual RecorderPtr operator()(uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f /* define buffer size for duration second */) = 0;
			};
			FLAME_SOUND_EXPORTS static Create& create;
		};
	}
}
