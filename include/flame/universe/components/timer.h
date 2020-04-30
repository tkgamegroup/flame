#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTimer : Component
	{
		float interval;

		float _time;

		cTimer() :
			Component("cTimer")
		{
		}

		FLAME_UNIVERSE_EXPORTS void start(bool force_restart = true);
		FLAME_UNIVERSE_EXPORTS void stop();
		FLAME_UNIVERSE_EXPORTS void reset();
		FLAME_UNIVERSE_EXPORTS void set_callback(void(*callback)(Capture& c), const Capture& capture, bool start = true);

		FLAME_UNIVERSE_EXPORTS static cTimer* create();
	};
}
