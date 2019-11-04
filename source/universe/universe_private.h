#pragma once

#include <flame/universe/universe.h>
#include "world_private.h"

namespace flame
{
	struct ListenerHub
	{
		std::vector<std::unique_ptr<Closure<void(void* c)>>> listeners;
	};

	struct UniversePrivate : Universe
	{
		std::vector<Object*> objects;

		std::vector<std::unique_ptr<WorldPrivate>> worlds;

		void update();
	};
}
