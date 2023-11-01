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

			~SourcePrivate();

			void add_buffer(BufferPtr buffer) override;
			void remove_buffer(BufferPtr buffer) override;
			bool is_player() const override;
			void play() override;
			void stop() override;
			void pause() override;
			void rewind() override;
			void set_volumn(float v) override;
		};
	}
}
