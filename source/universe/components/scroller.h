#pragma once

#include "../component.h"

namespace flame
{
	struct cScroller : Component
	{
		inline static auto type_name = "flame::cScroller";
		inline static auto type_hash = ch(type_name);

		cScroller() : Component(type_name, type_hash)
		FLAME_UNIVERSE_EXPORTS static cScroller* create(void* parms = nullptr);
	};
}
