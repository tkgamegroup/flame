#include "entity_private.h"
#include <flame/universe/world.h>

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

	Entity* World::root() const
	{
		return ((WorldPrivate*)this)->root.get();
	}

	void World::add_system(System* s)
	{
		((WorldPrivate*)this)->systems.emplace_back(s);
	}

	World* World::create()
	{
		return new WorldPrivate;
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}
}
