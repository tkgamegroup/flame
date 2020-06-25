#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "universe_private.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::vector<Object*> _objects;

		std::vector<std::unique_ptr<System, Delecter>> _systems;
		std::unique_ptr<EntityPrivate> _root;
	};
}
