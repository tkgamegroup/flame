#pragma once

#include "sound.h"

namespace flame
{
	namespace sound
	{
		struct Source
		{
			virtual ~Source() {}

			virtual void set_volume(float v) = 0;
			virtual void set_looping(bool v) = 0;
			virtual void play() = 0;
			virtual void stop() = 0;

			struct Create
			{
				virtual SourcePtr operator()(BufferPtr buffer) = 0;
			};
			FLAME_SOUND_EXPORTS static Create& create;
		};
	}
}
