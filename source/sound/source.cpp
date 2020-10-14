#include "source_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace sound
	{
		SourcePrivate::SourcePrivate(BufferPrivate* buffer)
		{
			alGenSources(1, &al_src);
			alSourcei(al_src, AL_BUFFER, buffer->al_buf);
		}

		SourcePrivate::~SourcePrivate()
		{
			alDeleteSources(1, &al_src);
		}

		void SourcePrivate::set_volume(float v) 
		{
			alSourcef(al_src, AL_GAIN, v);
		}

		void SourcePrivate::set_looping(bool v)
		{
			alSourcei(al_src, AL_LOOPING, v);
		}

		void SourcePrivate::play()
		{
			ALint state;
			alGetSourcei(al_src, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING)
			{
				alSourceStop(al_src);
				alSourcePlay(al_src);
			}
		}

		void SourcePrivate::stop()
		{
			alSourceStop(al_src);
		}

		Source *Source::create(Buffer* buffer)
		{
			return new SourcePrivate((BufferPrivate*)buffer);
		}
	}
}
