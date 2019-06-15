#pragma once

#include <flame/sound/source.h>
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct SourcePrivate : Source
		{
			ALuint al_src;

			SourcePrivate(Buffer *b);
			~SourcePrivate();

			void set_volume(float v);
			void set_looping(bool v);
			void play();
			void stop();
		};
	}
}
