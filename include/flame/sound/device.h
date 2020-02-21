#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Device
		{
			FLAME_SOUND_EXPORTS void start_record();
			FLAME_SOUND_EXPORTS void stop_record(void* dst);

			FLAME_SOUND_EXPORTS static Device* create_player();
			FLAME_SOUND_EXPORTS static Device* create_recorder(uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f /* define buffer size for duration second */);
			FLAME_SOUND_EXPORTS static void destroy(Device* d);
		};
	}
}
