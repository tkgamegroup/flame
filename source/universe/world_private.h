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

		WorldPrivate()
		{
			auto e = new EntityPrivate;
			e->world_ = this;
			root.reset(e);
		}
	};
}
