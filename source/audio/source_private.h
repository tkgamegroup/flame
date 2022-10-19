#pragma once

#include "source.h"
#include "audio_private.h"

namespace flame
{
	namespace audio
	{
		struct SourcePrivate : Source
		{
			ALuint al_src = 0;

			void set_pos(const vec3& pos) override;

			void play() override;
			void stop() override;
		};
	}
}
