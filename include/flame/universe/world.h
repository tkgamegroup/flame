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

		FLAME_UNIVERSE_EXPORTS void add_system(System* s);

		FLAME_UNIVERSE_EXPORTS static World* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(World* w);
	};
}
