#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::unique_ptr<EntityPrivate> root;
		std::vector<std::unique_ptr<System>> systems;

		WorldPrivate();
		System* get_system_plain(uint name_hash) const;
	};
}
