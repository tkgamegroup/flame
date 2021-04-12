#pragma once

#include "../driver.h"

namespace flame
{
	struct dCheckbox : Driver
	{
		inline static auto type_name = "flame::dCheckbox";
		inline static auto type_hash = ch(type_name);

		dCheckbox() :
			Driver(type_name, type_hash)
		{
		}

		virtual bool get_checked() const = 0;
		virtual void set_checked(bool checked) = 0;

		FLAME_UNIVERSE_EXPORTS static dCheckbox* create(void* parms = nullptr);
	};
}
