#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dMenu : Driver
	{
		inline static auto type_name = "flame::dMenu";
		inline static auto type_hash = ch(type_name);

		dMenu() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dMenu* create();
	};

	struct dMenuItem : Driver
	{
		inline static auto type_name = "flame::dMenuItem";
		inline static auto type_hash = ch(type_name);

		dMenuItem() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dMenuItem* create();
	};

	struct dMenuBar : Driver
	{
		inline static auto type_name = "flame::dMenuBar";
		inline static auto type_hash = ch(type_name);

		dMenuBar() :
			Driver(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dMenuBar* create();
	};
}
