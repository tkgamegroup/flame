#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTimer : Component
	{
		float _t;
		float interval;

		ListenerHub<bool(void* c)> callbacks;

		cTimer() :
			Component("cTimer")
		{
		}

		FLAME_UNIVERSE_EXPORTS void start();
		FLAME_UNIVERSE_EXPORTS void stop();

		FLAME_UNIVERSE_EXPORTS static cTimer* create();
	};
}
