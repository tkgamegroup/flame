#pragma once

#include <flame/sound/sound.h>

#include <string>

namespace flame
{
	namespace sound
	{
		struct Buffer
		{
			FLAME_SOUND_EXPORTS static Buffer* create_from_data(int size, void* data);
			FLAME_SOUND_EXPORTS static Buffer *create_from_file(const std::wstring& filename, bool reverse = false);
			FLAME_SOUND_EXPORTS static void destroy(Buffer *b);
		};
	}
}
