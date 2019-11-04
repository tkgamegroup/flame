#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		universe_ = nullptr;

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

	void World::add_object(Object* o, const std::string& id)
	{
		((WorldPrivate*)this)->objects.emplace_back(o, id);
	}

	Object* World::find_object(uint name_hash, const std::string& id)
	{
		const auto& objects = ((WorldPrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first->name_hash == name_hash)
			{
				if (!id.empty() && o.second == id)
					return o.first;
			}
		}
		return universe_ ? universe_->find_object(name_hash, id) : nullptr;
	}

	const std::string* World::find_id(Object* _o)
	{
		const auto& objects = ((WorldPrivate*)this)->objects;
		for (auto& o : objects)
		{
			if (o.first == _o)
				return &o.second;
		}
		return universe_ ? universe_->find_id(_o) : nullptr;
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

	World* World::create()
	{
		return new WorldPrivate;
	}

	void World::destroy(World* w)
	{
		delete (WorldPrivate*)w;
	}
}
