#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Device;

		struct Context
		{
			FLAME_SOUND_EXPORTS void make_current();

			FLAME_SOUND_EXPORTS static Context *create(Device *d);
			FLAME_SOUND_EXPORTS static void destroy(Context *c);
		};
	}
}
