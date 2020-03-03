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

		FLAME_UNIVERSE_EXPORTS void add_stringa_item(uint hash, const char* v);
		FLAME_UNIVERSE_EXPORTS const char* get_stringa_item(uint hash);
		FLAME_UNIVERSE_EXPORTS void remove_stringa_item(uint hash);

		FLAME_UNIVERSE_EXPORTS static cDataKeeper* create();
	};
}
