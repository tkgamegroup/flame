#pragma once

#include "audio.h"

namespace flame
{
	namespace audio
	{
		struct Source
		{
			vec3 pos = vec3(0.f);
			virtual void set_pos(const vec3& pos) = 0;

			bool auto_replay = false;

			virtual ~Source() {}

			virtual void add_buffer(BufferPtr buffer) = 0;
			virtual void remove_buffer(BufferPtr buffer) = 0;
			virtual void set_loop(bool v) = 0;
			virtual bool is_player() const = 0;
			virtual void play() = 0;
			virtual void stop() = 0;
			virtual void pause() = 0;
			virtual void rewind() = 0;
			virtual void set_volumn(float v) = 0;

			struct Create
			{
				virtual SourcePtr operator()() = 0;
			};
			FLAME_AUDIO_API static Create& create;
		};
	}
}
