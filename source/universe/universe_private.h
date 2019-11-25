#pragma once

#include <flame/universe/universe.h>
#include "world_private.h"

namespace flame
{
	struct UniversePrivate : Universe
	{
		std::vector<Object*> objects;

		std::vector<std::unique_ptr<WorldPrivate>> worlds;

		void update();
	};
}
