#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cDataKeeper : Component
	{
		cDataKeeper() :
			Component("cDataKeeper")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_common_item(uint hash, const CommonValue& v);
		FLAME_UNIVERSE_EXPORTS CommonValue get_common_item(uint hash);
		FLAME_UNIVERSE_EXPORTS void remove_common_item(uint hash);

		FLAME_UNIVERSE_EXPORTS void set_string_item(uint hash, const char* v);
		FLAME_UNIVERSE_EXPORTS const char* get_string_item(uint hash);
		FLAME_UNIVERSE_EXPORTS void remove_string_item(uint hash);

		FLAME_UNIVERSE_EXPORTS static cDataKeeper* create();
	};
}
