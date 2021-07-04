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

	System* WorldPrivate::get_system(uint type_hash) const
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
		update_list.resize(systems.size());
		for (auto i = 0; i < systems.size(); i++)
			update_list[i] = systems[i].get();
	}

	void WorldPrivate::remove_system(System* s)
	{
		for (auto it = systems.begin(); it != systems.end(); it++)
		{
			if ((*it)->type_hash == s->type_hash)
			{
				systems.erase(it);
				update_list.resize(systems.size());
				for (auto i = 0; i < systems.size(); i++)
					update_list[i] = systems[i].get();
				return;
			}
		}
	}

	void WorldPrivate::set_update_list(uint count, uint* indices)
	{
		update_list.resize(count);
		for (auto i = 0; i < count; i++)
			update_list[i] = systems[indices[i]].get();
	}

	void WorldPrivate::update()
	{
		for (auto s : update_list)
		{
			for (auto& l : update_listeners)
				l->call(s, true);
			s->update();
			for (auto& l : update_listeners)
				l->call(s, false);
		}
	}

	void* WorldPrivate::add_update_listener(void (*callback)(Capture& c, System* system, bool before), const Capture& capture)
	{
		auto c = new Closure(callback, capture);
		update_listeners.emplace_back(c);
		return c;
	}

	void WorldPrivate::remove_update_listener(void* ret)
	{
		std::erase_if(update_listeners, [&](const auto& i) {
			return i == (decltype(i))ret;
		});
	}

	World* World::create()
	{
		return new WorldPrivate;
	}
}
