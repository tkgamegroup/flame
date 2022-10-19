#pragma once

#include "audio_listener.h"

namespace flame
{
	struct cAudioListenerPrivate : cAudioListener
	{
		void on_active() override;
		void on_inactive() override;
		void update() override;
	};
}
