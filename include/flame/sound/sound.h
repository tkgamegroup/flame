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
		inline uint get_samples(float duration, uint frequency)
		{
			return frequency * duration;
		}

		inline uint get_size(float duration, uint frequency, bool stereo, bool _16bit)
		{
			return get_samples(duration, frequency) * (stereo ? 2 : 1) * (_16bit ? 2 : 1);
		}

		inline uint get_size(uint samples, bool stereo, bool _16bit)
		{
			return samples * (stereo ? 2 : 1) * (_16bit ? 2 : 1);
		}
	}
}
