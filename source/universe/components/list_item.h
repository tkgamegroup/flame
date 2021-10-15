#pragma once

#include "../component.h"

namespace flame
{
	struct cListItem : Component
	{
		inline static auto type_name = "flame::cListItem";
		inline static auto type_hash = ch(type_name);

		cListItem() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create(void* parms = nullptr);
	};
}
