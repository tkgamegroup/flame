#pragma once

#include <flame/universe/driver.h>

namespace flame
{
	struct dCombobox : Driver
	{
		inline static auto type_name = "flame::dCombobox";
		inline static auto type_hash = ch(type_name);

		dCombobox() :
			Driver(type_name, type_hash)
		{
		}

		virtual int get_index() const = 0;
		virtual void set_index(int index) = 0;

		FLAME_UNIVERSE_EXPORTS static dCombobox* create();
	};
}
