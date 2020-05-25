#include <flame/foundation/typeinfo.h>
#include <flame/universe/entity.h>
#include <flame/universe/world.h>

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(new EntityPrivate);
		root->world = this;
		root->global_visibility = true;
	}

	WorldPrivate::~WorldPrivate()
	{
		root->mark_dying();
	}

	System* WorldPrivate::get_system_plain(uint name_hash) const
	{
		for (auto& s : systems)
		{
			if (s->name_hash == name_hash)
				return s.get();
		}
		return nullptr;
	}

	void WorldPrivate::update()
	{
		for (auto& s : systems)
		{
			s->before_update();
			s->before_update_listeners.call();
			s->update();
			s->after_update_listeners.call();
			s->after_update();
		}
	}

	System* World::get_system_plain(uint name_hash) const
	{
		return ((WorldPrivate*)this)->get_system_plain(name_hash);
	}

	void World::add_system(System* s)
	{
		s->world_ = this;
		((WorldPrivate*)this)->systems.emplace_back(s);
		s->on_added();
	}

	Entity* World::root() const
	{
		return ((WorldPrivate*)this)->root.get();
	}

	void World::update()
	{
		((WorldPrivate*)this)->update();
	}

	World* World::create()
	{
		return new WorldPrivate();
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}
}
