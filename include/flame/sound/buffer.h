#pragma once

#include <flame/sound/sound.h>

namespace flame
{
	namespace sound
	{
		struct Buffer
		{
			virtual void release() = 0;

			FLAME_SOUND_EXPORTS static Buffer* create(void* data, uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f);
			FLAME_SOUND_EXPORTS static Buffer* create(const wchar_t* filename);
		};
	}
}
