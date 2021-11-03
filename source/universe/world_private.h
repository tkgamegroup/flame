#pragma once

#include "world.h"
#include "system.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::vector<std::unique_ptr<System>> systems;
		std::unique_ptr<EntityPrivate> root;
		EntityPrivate* first_element = nullptr;
		EntityPrivate* first_node = nullptr;
		EntityPrivate* first_imgui = nullptr;

		WorldPrivate();

		void release() override { delete this; }

		System* get_system(uint type_hash) const override;
		System* find_system(const std::string& name) const;
		System* find_system(const char* name) const override { return find_system(std::string(name)); }
		void add_system(System* s) override;
		void remove_system(System* s) override;

		Entity* get_root() override { return root.get(); }

		void update() override;
	};
}
