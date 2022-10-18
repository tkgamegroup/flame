#pragma once

#include "audio.h"

namespace flame
{
	namespace audio
	{
		struct Device
		{
			virtual ~Device() {}

			struct Create
			{
				virtual DevicePtr operator()() = 0;
			};
			FLAME_AUDIO_API static Create& create;

			struct Current
			{
				virtual DevicePtr& operator()() = 0;
			};
			FLAME_AUDIO_API static Current& current;
		};
	}
}
