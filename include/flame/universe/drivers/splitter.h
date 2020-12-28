#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dSplitter : Driver
	{
		inline static auto type_name = "flame::dSplitter";
		inline static auto type_hash = ch(type_name);

		dSplitter() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dSplitter* create();
	};
}
