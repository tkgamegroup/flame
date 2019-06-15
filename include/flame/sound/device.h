#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		enum DeviceType
		{
			DevicePlay,
			DeviceRecord
		};

		struct Device
		{
			FLAME_SOUND_EXPORTS void start_record();
			FLAME_SOUND_EXPORTS int get_recorded_samples();
			FLAME_SOUND_EXPORTS void get_recorded_data(void* dst, int samples);
			FLAME_SOUND_EXPORTS void stop_record();

			FLAME_SOUND_EXPORTS static Device* create(DeviceType t);
			FLAME_SOUND_EXPORTS static void destroy(Device* d);
		};
	}
}
