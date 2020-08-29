#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cBloom : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cBloom";
		inline static auto type_hash = ch(type_name);

		cBloom() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cBloom* create();
	};
}
