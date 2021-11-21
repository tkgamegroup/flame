#pragma once

#include "sound.h"

namespace flame
{
	namespace sound
	{
		struct Buffer
		{
			virtual ~Buffer() {}

			struct Create
			{
				virtual BufferPtr operator()(void* data, uint frequency = 44100, bool stereo = true, bool _16bit = true, float duration = 1.f) = 0;
				virtual BufferPtr operator()(const std::filesystem::path& filename) = 0;
			};
			FLAME_SOUND_EXPORTS static Create& create;
		};
	}
}
