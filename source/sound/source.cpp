#pragma once

#include "source_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace sound
	{
		SourcePrivate::SourcePrivate(Buffer *b)
		{
			alGenSources(1, &al_src);
			alSourcei(al_src, AL_BUFFER, ((BufferPrivate*)b)->al_buf);
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

		void Source::set_volume(float v)
		{
			((SourcePrivate*)this)->set_volume(v);
		}

		void Source::set_looping(bool v)
		{
			((SourcePrivate*)this)->set_looping(v);
		}

		void Source::play()
		{
			((SourcePrivate*)this)->play();
		}

		void Source::stop()
		{
			((SourcePrivate*)this)->stop();
		}

		Source *Source::create(Buffer *b)
		{
			return new SourcePrivate(b);
		}

		void Source::destroy(Source *s)
		{
			delete (SourcePrivate*)s;
		}
	}
}
