#pragma once

#include "../component.h"

namespace flame
{
	struct cMenuItem : Component
	{
		inline static auto type_name = "flame::cMenuItem";
		inline static auto type_hash = ch(type_name);

		cMenuItem() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cMenuItem* create(void* parms = nullptr);
	};
}
