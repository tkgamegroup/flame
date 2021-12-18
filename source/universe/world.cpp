#include "../foundation/typeinfo.h"
#include "entity_private.h"
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(new EntityPrivate);
		root->world = this;
		root->global_visibility = true;
	}

	void WorldPrivate::add_system(System* s)
	{
		assert(!s->world);

		s->world = this;

		systems.emplace(s->type_hash, s);
		system_list.push_back(s);

		s->on_added();
	}

	void WorldPrivate::remove_system(System* s, bool destroy)
	{
		assert(s->world == this);

		auto it = systems.find(s->type_hash);
		if (it == systems.end())
		{
			assert(0);
			return;
		}

		it->second.release();
		systems.erase(it);

		for (auto it = system_list.begin(); it != system_list.end(); it++)
		{
			if (*it == s)
			{
				system_list.erase(it);
				break;
			}
		}
	}

	void WorldPrivate::update()
	{
		for (auto s : system_list)
			s->update();
	}

	World* World::create()
	{
		return new WorldPrivate;
	}
}
