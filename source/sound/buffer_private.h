#pragma once

#include "buffer.h"
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct BufferPrivate : Buffer
		{
			ALuint al_buf;

			~BufferPrivate();
		};
	}
}
