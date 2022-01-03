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

	System* WorldPrivate::add_system(uint hash)
	{
		auto ui = find_udt(hash);
		if (!ui)
		{
			printf("cannot add system: cannot find udt of hash %u\n", hash);
			return nullptr;
		}

		if (system_map.find(hash) != system_map.end())
		{
			printf("cannot add system: %s already exist\n", ui->name.c_str());
			return nullptr;
		}

		auto fi = ui->find_function("create");
		if (!fi)
		{
			printf("cannot add system: cannot find create function of %s\n", ui->name.c_str());
			return nullptr;
		}

		if (fi->return_type != TypeInfo::get(TagPU, ui->name) || fi->parameters.size() != 1 || fi->parameters[0] != TypeInfo::get<World*>())
		{
			printf("cannot add system: %s's create function format does not match\n", ui->name.c_str());
			return nullptr;
		}

		auto s = fi->call<System* (void*, World*)>(nullptr, this);
		if (!s)
		{
			printf("cannot add system: %s's create funcion returned nullptr\n", ui->name.c_str());
			return nullptr;
		}

		s->type_hash = hash;
		s->world = this;

		system_map.emplace(s->type_hash, s);
		systems.emplace_back(s);

		return s;
	}

	void WorldPrivate::remove_system(uint hash, bool destroy)
	{
		auto it = system_map.find(hash);
		if (it == system_map.end())
		{
			printf("cannot remove sytem: system with hash %u does not exist", hash);
			return;
		}

		auto s = it->second;
		system_map.erase(it);

		for (auto it = systems.begin(); it != systems.end(); it++)
		{
			if (it->get() == s)
			{
				it->release();
				systems.erase(it);
				break;
			}
		}

		if (destroy)
			delete s;
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
