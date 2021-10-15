#pragma once

#include "../component.h"

namespace flame
{
	struct cWindow : Component
	{
		inline static auto type_name = "flame::cWindow";
		inline static auto type_hash = ch(type_name);

		cWindow() : Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cWindow* create(void* parms = nullptr);
	};
}
