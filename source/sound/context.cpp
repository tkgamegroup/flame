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
