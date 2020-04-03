#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTimer : Component
	{
		float interval;
		float max_time;
		int max_times;

		bool _updating;
		float _time;
		float _total_time;
		int _times;

		cTimer() :
			Component("cTimer")
		{
		}

		FLAME_UNIVERSE_EXPORTS void start(bool force_restart = true);
		FLAME_UNIVERSE_EXPORTS void stop();
		FLAME_UNIVERSE_EXPORTS void reset();
		FLAME_UNIVERSE_EXPORTS void set_callback(void(*callback)(void* c), const Mail& capture, bool start = true);

		FLAME_UNIVERSE_EXPORTS static cTimer* create();
	};
}
