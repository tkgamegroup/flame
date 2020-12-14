#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dWindow : Driver
	{
		inline static auto type_name = "flame::dWindow";
		inline static auto type_hash = ch(type_name);

		dWindow() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dWindow* create();
	};
}