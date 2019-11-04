#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::vector<std::pair<Object*, std::string>> objects;

		std::vector<std::unique_ptr<System>> systems;

		std::unique_ptr<EntityPrivate> root;

		WorldPrivate();
		System* get_system_plain(uint name_hash) const;
	};
}
