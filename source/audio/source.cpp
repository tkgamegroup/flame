#include "source_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace audio
	{
		void SourcePrivate::set_pos(const vec3& _pos)
		{
			if (pos == _pos)
				return;
			pos = _pos;
			alSourcefv(al_src, AL_POSITION, &pos[0]);
		}

		SourcePrivate::~SourcePrivate()
		{
			alDeleteSources(1, &al_src);
		}

		void SourcePrivate::add_buffer(BufferPtr buffer)
		{
			alSourceQueueBuffers(al_src, 1, &buffer->al_buf);
		}

		void SourcePrivate::remove_buffer(BufferPtr buffer)
		{
			alSourceUnqueueBuffers(al_src, 1, &buffer->al_buf);
		}

		bool SourcePrivate::is_player() const
		{
			ALenum state;
			alGetSourcei(al_src, AL_SOURCE_STATE, &state);
			return state == AL_PLAYING;
		}

		void SourcePrivate::play()
		{
			ALenum state;
			alGetSourcei(al_src, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING)
				alSourcePlay(al_src);
		}

		void SourcePrivate::stop()
		{
			alSourceStop(al_src);
		}

		void SourcePrivate::pause()
		{
			alSourcePause(al_src);
		}

		void SourcePrivate::rewind()
		{
			alSourceRewind(al_src);
		}

		void SourcePrivate::set_volumn(float v)
		{
			alSourcef(al_src, AL_GAIN, v);
		}

		struct SourceCreate : Source::Create
		{
			SourcePtr operator()() override
			{
				auto ret = new SourcePrivate;
				alGenSources(1, &ret->al_src);
				return ret;
			}
		}Source_create;
		Source::Create& Source::create = Source_create;
	}
}
