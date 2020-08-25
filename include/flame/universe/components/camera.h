#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cCamera : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cCamera";
		inline static auto type_hash = ch(type_name);

		cCamera() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cCamera* create();
	};
}
