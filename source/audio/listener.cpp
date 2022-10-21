#include "listener_private.h"

namespace flame
{
	namespace audio
	{
		void ListenerPrivate::set_pos(const vec3& _pos)
		{
			if (pos == _pos)
				return;
			pos = _pos;
			alListenerfv(AL_POSITION, &pos[0]);
		}

		struct ListenerGet : Listener::Get
		{
			ListenerPtr operator()() override
			{
				static ListenerPtr listener = nullptr;
				if (!listener)
					listener = new ListenerPrivate;
				return listener;
			}
		}Listener_get;
		Listener::Get& Listener::get = Listener_get;
	}
}
