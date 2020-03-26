#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTimer : Component
	{
		float _t;
		float interval;
		int times;
		float duration;

		cTimer() :
			Component("cTimer")
		{
		}

		FLAME_UNIVERSE_EXPORTS void start();
		FLAME_UNIVERSE_EXPORTS void stop();
		FLAME_UNIVERSE_EXPORTS void set_callback(void(*callback)(void* c), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS static cTimer* create();
	};
}
