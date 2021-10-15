#pragma once

#include "../component.h"

namespace flame
{
	struct cGrid : Component
	{
		inline static auto type_name = "flame::cGrid";
		inline static auto type_hash = ch(type_name);

		cGrid() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cGrid* create(void* parms = nullptr);
	};
}
