#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cDragMove : Component
	{
		inline static auto type_name = "flame::cDragMove";
		inline static auto type_hash = ch(type_name);

		cDragMove() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cDragMove* create();
	};
}
