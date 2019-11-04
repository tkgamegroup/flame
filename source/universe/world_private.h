#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "entity_private.h"

namespace flame
{
	struct UdtInfo;

	struct WorldPrivate : World
	{
		std::wstring filename;

		std::vector<std::pair<Object*, UdtInfo*>> objects;

		std::vector<std::unique_ptr<System>> systems;

		std::unique_ptr<EntityPrivate> root;

		WorldPrivate(Universe* u);
		~WorldPrivate();
		System* get_system_plain(uint name_hash) const;
	};
}
