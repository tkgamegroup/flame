#pragma once

#include "../driver.h"

namespace flame
{
	struct dList : Driver
	{
		inline static auto type_name = "flame::dList";
		inline static auto type_hash = ch(type_name);

		dList() :
			Driver(type_name, type_hash)
		{
		}

		virtual EntityPtr get_selected() const = 0;
		virtual void set_selected(EntityPtr e) = 0;

		FLAME_UNIVERSE_EXPORTS static dList* create(void* parms = nullptr);
	};
}
