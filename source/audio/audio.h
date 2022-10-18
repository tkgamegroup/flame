#pragma once

#include "../foundation/foundation.h"

#ifdef FLAME_AUDIO_MODULE

#define FLAME_AUDIO_API __declspec(dllexport)

#define FLAME_AUDIO_TYPE(name) FLAME_TYPE_PRIVATE(name)

#else

#define FLAME_AUDIO_API __declspec(dllimport)

#define FLAME_AUDIO_TYPE(name) FLAME_TYPE(name)

#endif

namespace flame
{
	namespace audio
	{
		FLAME_AUDIO_TYPE(Device);
		FLAME_AUDIO_TYPE(Buffer);
		FLAME_AUDIO_TYPE(Source);
		FLAME_AUDIO_TYPE(Listener);
	}
}
