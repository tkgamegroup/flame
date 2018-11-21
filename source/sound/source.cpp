// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include "source_private.h"
#include "buffer_private.h"

namespace flame
{
	namespace sound
	{
		inline SourcePrivate::SourcePrivate(Buffer *b)
		{
			alGenSources(1, &al_src);
			alSourcei(al_src, AL_BUFFER, ((BufferPrivate*)b)->al_buf);
		}

		inline SourcePrivate::~SourcePrivate()
		{
			alDeleteSources(1, &al_src);
		}

		inline void SourcePrivate::set_volume(float v) 
		{
			alSourcef(al_src, AL_GAIN, v);
		}

		inline void SourcePrivate::set_looping(bool v)
		{
			alSourcei(al_src, AL_LOOPING, v);
		}

		inline void SourcePrivate::play()
		{
			alSourcePlay(al_src);
		}

		inline void SourcePrivate::stop()
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
