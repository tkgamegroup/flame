#pragma once

#include <flame/universe/world.h>
#include <flame/universe/system.h>
#include "entity_private.h"

namespace flame
{
	struct WorldBridge : World
	{
		void register_object(void* o, const char* name) override;
		void* find_object(const char* name) const override;
	};

	struct WorldPrivate : WorldBridge
	{
		std::vector<std::pair<void*, std::string>> objects;

		std::vector<std::unique_ptr<System, Delector>> systems;
		std::unique_ptr<EntityPrivate, Delector> root;

		WorldPrivate();

		void release() override { delete this; }

		void register_object(void* o, const std::string& name);
		void* find_object(const std::string& name) const;

		System* get_system(uint64 type_hash) const override;
		void add_system(System* s) override;
		void remove_system(System* s) override;

		Entity* get_root() const override { return root.get(); }

		void update() override;
	};

	inline void WorldBridge::register_object(void* o, const char* name)
	{
		((WorldPrivate*)this)->register_object(o, name);
	}

	inline void* WorldBridge::find_object(const char* name) const
	{
		return ((WorldPrivate*)this)->find_object(name);
	}
}
