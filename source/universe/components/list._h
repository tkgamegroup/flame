#pragma once

#include "../component.h"

namespace flame
{
	struct cList : Component
	{
		inline static auto type_name = "flame::cList";
		inline static auto type_hash = ch(type_name);
		
		cList() : Component(type_name, type_hash)
		{
		}
		
		virtual EntityPtr get_selected() const = 0;
		virtual void set_selected(EntityPtr v) = 0;
		
		FLAME_UNIVERSE_EXPORTS static cList* create(void* parms = nullptr);
	};
}
