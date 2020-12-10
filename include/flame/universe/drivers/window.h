#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dWindow : Driver
	{
		inline static auto type_name = "flame::dWindow";

		dWindow() :
			Driver(type_name)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dWindow* create();
	};
}
