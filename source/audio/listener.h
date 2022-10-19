#pragma once

#include "audio.h"

namespace flame
{
	namespace audio
	{
		struct Listener
		{
			vec3 pos = vec3(0.f);
			virtual void set_pos(const vec3& pos) = 0;

			virtual ~Listener() {}

			struct Get
			{
				virtual ListenerPtr operator()() = 0;
			};
			FLAME_AUDIO_API static Get& get;
		};
	}
}
