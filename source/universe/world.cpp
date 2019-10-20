#include "world_private.h"

namespace flame
{
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
