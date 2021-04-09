#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Source
		{
			virtual void release() = 0;

			virtual void set_volume(float v) = 0;
			virtual void set_looping(bool v) = 0;
			virtual void play() = 0;
			virtual void stop() = 0;

			FLAME_SOUND_EXPORTS static Source* create(Buffer* buffer);
		};
	}
}
