#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cDragResize : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cDragResize";
		inline static auto type_hash = ch(type_name);

		cDragResize() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cDragResize* create();
	};
}
