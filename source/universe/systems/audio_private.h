#pragma once

#include "audio.h"

#ifdef USE_OPENAL
#endif

namespace flame
{
	struct sAudioPrivate : sAudio
	{
		sAudioPrivate();
		sAudioPrivate(int); // dummy
		~sAudioPrivate();

		void play_once(const std::filesystem::path& path) override;
		void update() override;
	};
}
