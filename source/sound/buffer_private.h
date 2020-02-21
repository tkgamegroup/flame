#pragma once

#include <flame/sound/buffer.h>
#include "sound_private.h"

#include <fstream>

namespace flame
{
	namespace sound
	{
		struct BufferPrivate : Buffer
		{
			ALuint al_buf;

			BufferPrivate(void* data, uint frequency, bool stereo, bool _16bit, float duration);
			BufferPrivate(FILE* f);
			~BufferPrivate();
		};
	}
}
