#include "../foundation/typeinfo.h"
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(new EntityPrivate());
		root->world = this;
		root->global_visibility = true;
	}

	void WorldPrivate::add_system(System* s)
	{
		assert(!s->world);
		s->world = this;
		systems.emplace_back(s);
		s->on_added();
	}

	void WorldPrivate::remove_system(System* s)
	{
		for (auto it = systems.begin(); it != systems.end(); it++)
		{
			if ((*it)->type_hash == s->type_hash)
			{
				systems.erase(it);
				return;
			}
		}
	}

	void WorldPrivate::update()
	{
		for (auto& s : systems)
			s->update();
	}

	World* World::create()
	{
		return new WorldPrivate;
	}
}
