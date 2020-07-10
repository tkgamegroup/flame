#include <flame/foundation/typeinfo.h>
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(EntityPrivate::create());
		root->world = this;
		root->global_visibility = true;
	}

	void WorldBridge::register_object(void* o, const char* name)
	{ 
		((WorldPrivate*)this)->register_object(o, name);
	}

	void WorldPrivate::register_object(void* o, const std::string& name)
	{
		objects.emplace_back(o, name);
	}

	void* WorldBridge::find_object(const char* name) const
	{
		return ((WorldPrivate*)this)->find_object(name);
	}

	void* WorldPrivate::find_object(const std::string& name) const
	{
		for (auto& o : objects)
		{
			if (o.second == name)
				return o.first;
		}
		return nullptr;
	}

	System* WorldPrivate::get_system(uint name_hash) const
	{
		for (auto& s : systems)
		{
			if (s->type_hash == name_hash)
				return s.get();
		}
		return nullptr;
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
