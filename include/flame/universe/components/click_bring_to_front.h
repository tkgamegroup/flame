#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cClickBringToFront : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cClickBringToFront";
		inline static auto type_hash = ch(type_name);

		cClickBringToFront() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cClickBringToFront* create();
	};
}
