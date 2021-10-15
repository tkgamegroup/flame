#pragma once

#include "../component.h"

namespace flame
{
	struct cMenu : Component
	{
		inline static auto type_name = "flame::cMenu";
		inline static auto type_hash = ch(type_name);

		cMenu() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cMenu* create(void* parms = nullptr);
	};
}
