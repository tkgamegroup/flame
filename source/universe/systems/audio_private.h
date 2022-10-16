#pragma once

#include "audio.h"

#ifdef USE_OPENAL
#endif

namespace flame
{
	struct sAudioPrivate : sAudio
	{
		int get_buffer_res(const std::filesystem::path& filename, int id = -1) override;
		void release_buffer_res(uint id) override;

		void update() override;
	};
}
