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

			BufferPrivate(FILE* f, bool reverse = false);
			BufferPrivate(int size, void* data);
			~BufferPrivate();
		};
	}
}
