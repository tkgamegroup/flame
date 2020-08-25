#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cNode : Component // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cNode";
		inline static auto type_hash = ch(type_name);

		cNode() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static cNode* create();
	};
}
