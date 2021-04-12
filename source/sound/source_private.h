#pragma once

#include "source.h"
#include "sound_private.h"

namespace flame
{
	namespace sound
	{
		struct SourcePrivate : Source
		{
			ALuint al_src;

			SourcePrivate(BufferPrivate* buffer);
			~SourcePrivate();

			void release() override { delete this; }

			void set_volume(float v) override;
			void set_looping(bool v) override;
			void play() override;
			void stop() override;
		};
	}
}
