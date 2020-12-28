#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dGrid : Driver
	{
		inline static auto type_name = "flame::dGrid";
		inline static auto type_hash = ch(type_name);

		dGrid() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dGrid* create();
	};
}
