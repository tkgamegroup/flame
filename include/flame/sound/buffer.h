#pragma once

#include <flame/sound/sound.h>

#include <string>

namespace flame
{
	namespace sound
	{
		struct Buffer
		{
			FLAME_SOUND_EXPORTS static Buffer* create_from_data(void* data, uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f);
			FLAME_SOUND_EXPORTS static Buffer *create_from_file(const wchar_t* filename);
			FLAME_SOUND_EXPORTS static void destroy(Buffer *b);
		};
	}
}
