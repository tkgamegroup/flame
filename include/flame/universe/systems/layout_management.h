#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct cLayout;

	struct sLayoutManagement : System
	{
		sLayoutManagement() :
			System("LayoutManagement")
		{
		}

		FLAME_UNIVERSE_EXPORTS void add_to_update_list(cLayout* l);
		FLAME_UNIVERSE_EXPORTS void remove_from_update_list(cLayout* l);

		FLAME_UNIVERSE_EXPORTS static sLayoutManagement* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(sLayoutManagement* s);
	};
}
