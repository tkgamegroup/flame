#pragma once

#ifdef FLAME_SOUND_MODULE
#define FLAME_SOUND_EXPORTS __declspec(dllexport)
#else
#define FLAME_SOUND_EXPORTS __declspec(dllimport)
#endif

#include <flame/type.h>

namespace flame
{
	namespace sound
	{
		inline uint sound_size(float duration, uint frequency = 44100, bool stereo = true, bool _16bit = true)
		{
			return frequency * (stereo ? 2 : 1) * (_16bit ? 2 : 1) * duration;
		}
	}
}
