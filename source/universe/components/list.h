#pragma once

#include "../component.h"

namespace flame
{
	struct cList : Component
	{
		inline static auto type_name = "flame::cList";
		inline static auto type_hash = ch(type_name);

		cList() : Component(type_name, type_hash)
		FLAME_UNIVERSE_EXPORTS static cList* create(void* parms = nullptr);
	};
}
