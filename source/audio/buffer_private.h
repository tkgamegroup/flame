#pragma once

#include "buffer.h"
#include "audio_private.h"

namespace flame
{
	namespace audio
	{
		struct BufferPrivate : Buffer
		{
			ALuint al_buf = 0;
		};
	}
}
