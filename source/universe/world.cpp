#include <flame/foundation/typeinfo.h>
#include <flame/script/script.h>
#include "world_private.h"

namespace flame
{
	WorldPrivate::WorldPrivate()
	{
		root.reset(f_new<EntityPrivate>());
		root->world = this;
		root->global_visibility = true;

		element_root.reset(f_new<EntityPrivate>());
		element_root->world = this;
		element_root->global_visibility = true;

		node_root.reset(f_new<EntityPrivate>());
		node_root->world = this;
		node_root->global_visibility = true;
	}

	void WorldPrivate::register_object(void* o, const std::string& name)
	{
		objects.emplace_back(o, name);
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

	System* WorldPrivate::get_system(uint64 type_hash) const
	{
		for (auto& s : systems)
		{
			if (s->type_hash == type_hash)
				return s.get();
		}
		return nullptr;
	}

	System* WorldPrivate::find_system(const std::string& _name) const
	{
		System* ret = nullptr;
		auto name = _name;
		for (auto& s : systems)
		{
			if (s->type_name == name)
			{
				ret = s.get();
				break;
			}
		}
		name = "flame::" + _name;
		for (auto& s : systems)
		{
			if (s->type_name == name)
			{
				ret = s.get();
				break;
			}
		}
		if (ret)
		{
			auto script = script::Instance::get_default();
			script->push_string(name.c_str());
			script->set_global_name("__type__");
		}
		return ret;
	}

	void WorldPrivate::add_system(System* s)
	{
		fassert(!s->world);
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
