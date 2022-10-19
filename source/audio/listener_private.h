#pragma once

#include "listener.h"
#include "audio_private.h"

namespace flame
{
	namespace audio
	{
		struct ListenerPrivate : Listener
		{
			void set_pos(const vec3& pos) override;
		};
	}
}
