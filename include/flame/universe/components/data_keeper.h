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

		FLAME_UNIVERSE_EXPORTS void set_voidp_item(uint hash, void* v);
		FLAME_UNIVERSE_EXPORTS void* get_voidp_item(uint hash);
		FLAME_UNIVERSE_EXPORTS void remove_voidp_item(uint hash);

		FLAME_UNIVERSE_EXPORTS void set_stringa_item(uint hash, const char* v);
		FLAME_UNIVERSE_EXPORTS const char* get_stringa_item(uint hash);
		FLAME_UNIVERSE_EXPORTS void remove_stringa_item(uint hash);

		FLAME_UNIVERSE_EXPORTS static cDataKeeper* create();
	};
}
