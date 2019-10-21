#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		auto e = new EntityPrivate;
		e->world_ = this;
		root.reset(e);
	}

	System*  WorldPrivate::get_system_plain(uint name_hash) const
	{
		for (auto& s : systems)
		{
			if (s->name_hash == name_hash)
				return s.get();
		}
		return nullptr;
	}

	Entity* World::root() const
	{
		return ((WorldPrivate*)this)->root.get();
	}

	System* World::get_system_plain(uint name_hash) const
	{
		return ((WorldPrivate*)this)->get_system_plain(name_hash);
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
