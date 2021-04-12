#pragma once

#include "sound.h"

#include <al.h>
#include <alc.h>

namespace flame
{
	namespace sound
	{
		inline uint to_backend(bool stereo, bool _16bit)
		{
			if (stereo && _16bit)
				return AL_FORMAT_STEREO16;
			if (stereo && !_16bit)
				return AL_FORMAT_STEREO8;
			if (!stereo && _16bit)
				return AL_FORMAT_MONO16;
			if (!stereo && !_16bit)
				return AL_FORMAT_MONO8;
			return 0;
		}
	}
}
