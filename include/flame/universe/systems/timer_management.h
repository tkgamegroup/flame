#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cTimer;

	struct sTimerManagement : System
	{
		sTimerManagement() :
			System("sTimerManagement")
		{
		}

		FLAME_UNIVERSE_EXPORTS void add_to_update_list(cTimer* t);
		FLAME_UNIVERSE_EXPORTS void remove_from_update_list(cTimer* t);

		FLAME_UNIVERSE_EXPORTS static sTimerManagement* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(sTimerManagement* s);
	};
}
