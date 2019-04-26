// MIT License
// 
// Copyright (c) 2019 wjs
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

#include <flame/sound/context.h>
#include "device_private.h"

namespace flame
{
	namespace sound
	{
		struct ContextPrivate : Context
		{
			ALCcontext *al_ctx;

			ContextPrivate(Device *d)
			{
				al_ctx = alcCreateContext(((DevicePrivate*)d)->al_dev, nullptr);
			}

			~ContextPrivate()
			{
				alcDestroyContext(al_ctx);
			}

			void make_current()
			{
				alcMakeContextCurrent(al_ctx);
			}
		};

		void Context::make_current()
		{
			((ContextPrivate*)this)->make_current();
		}

		Context *Context::create(Device *d)
		{
			return new ContextPrivate(d);
		}

		void Context::destroy(Context *c)
		{
			delete (ContextPrivate*)c;
		}
	}
}
