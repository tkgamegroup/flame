#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cShape : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cShape";
		inline static auto type_hash = ch(type_name);

		cShape() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cShape* create();
	};
}
