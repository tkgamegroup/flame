#pragma once

#include "sound.h"

namespace flame
{
	namespace sound
	{
		struct Buffer
		{
			virtual ~Buffer() {}

			FLAME_SOUND_EXPORTS static BufferPtr create(void* data, uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f);
			FLAME_SOUND_EXPORTS static BufferPtr create(const std::filesystem::path& filename);
		};
	}
}
