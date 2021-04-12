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

			BufferPrivate();
			BufferPrivate(void* data, uint frequency, bool stereo, bool _16bit, float duration);
			~BufferPrivate();

			void release() override { delete this; }

			static BufferPrivate* create(const std::filesystem::path& filename);
		};
	}
}
