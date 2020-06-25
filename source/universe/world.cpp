#include <flame/foundation/typeinfo.h>
#include "world_private.h"

namespace flame
{
	World::World()
	{
		root = f_new<Entity>();
		root->world = this;
		root->global_visibility = true;
	}

	World::~World()
	{
		mark_dying(root);
		f_delete(root);
	}

	void World::update()
	{
		for (auto s : systems)
		{
			s->before_update();
			s->before_update_listeners.call();
			s->update();
			s->after_update_listeners.call();
			s->after_update();
		}
	}

	void World::add_system(System* s)
	{
		s->world_ = this;
		systems.push_back(s);
		s->on_added();
	}

	World* World::create()
	{
		return new World();
	}

	void World::destroy(World* w)
	{
		delete (World*)w;
	}
}
