#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct System;

	struct World
	{
		Universe* universe_;

		FLAME_UNIVERSE_EXPORTS Entity* root() const;

		FLAME_UNIVERSE_EXPORTS System* get_system_plain(uint name_hash) const;

		template<class T>
		T* get_system_t(uint name_hash) const
		{
			return (T*)get_system_plain(name_hash);
		}

#define get_system(T) get_system_t<s##T>(cH(#T))

		FLAME_UNIVERSE_EXPORTS void add_system(System* s);

		FLAME_UNIVERSE_EXPORTS static World* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(World* w);
	};
}
