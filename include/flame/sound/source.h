#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Buffer;

		struct Source
		{
			FLAME_SOUND_EXPORTS void set_volume(float v);
			FLAME_SOUND_EXPORTS void set_looping(bool v);
			FLAME_SOUND_EXPORTS void play();
			FLAME_SOUND_EXPORTS void stop();

			FLAME_SOUND_EXPORTS static Source *create(Buffer *b);
			FLAME_SOUND_EXPORTS static void destroy(Source *s);
		};
	}
}
