#include <flame/foundation/typeinfo.h>
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		_root.reset(EntityPrivate::_create());
		_root->_world = this;
		_root->_global_visibility = true;
	}

	void WorldPrivate::_register_object(Object* o)
	{
		_objects.push_back(o);
	}

	Object* WorldPrivate::_find_object(uint name_hash) const
	{
		for (auto o : _objects)
		{
			if (o->name_hash == name_hash)
				return o;
		}
		return nullptr;
	}

	System* WorldPrivate::_get_system(uint name_hash) const
	{
		for (auto& s : _systems)
		{
			if (s->name_hash == name_hash)
				return s.get();
		}
		return nullptr;
	}

	void WorldPrivate::_add_system(System* s)
	{
		assert(!s->world);
		s->world = this;
		_systems.emplace_back(s);
		s->on_added();
	}

	void WorldPrivate::_remove_system(System* s)
	{
	}

	void WorldPrivate::_update()
	{
		for (auto& s : _systems)
			s->update();
	}

	World* World::create()
	{
		return new WorldPrivate;
	}
}
