#pragma once

#include "audio.h"

#ifdef USE_OPENAL
#endif

namespace flame
{
	struct sAudioPrivate : sAudio
	{
		sAudioPrivate();
		~sAudioPrivate();
		void update() override;
	};
}
