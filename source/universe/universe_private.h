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
		std::vector<std::pair<Object*, std::string>> objects;

		std::vector<std::unique_ptr<WorldPrivate>> worlds;

		void update();
	};
}
