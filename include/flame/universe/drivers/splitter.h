#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct dSplitter : Component
	{
		inline static auto type_name = "flame::dSplitter";
		inline static auto type_hash = ch(type_name);

		dSplitter() :
			Component(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static dSplitter* create();
	};
}
