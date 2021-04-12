#pragma once

#include "world.h"
#include "system.h"
#include "entity_private.h"

namespace flame
{
	struct WorldPrivate : World
	{
		std::vector<std::pair<void*, std::string>> objects;

		std::vector<std::unique_ptr<System, Delector>> systems;
		std::unique_ptr<EntityPrivate, Delector> element_root;
		std::unique_ptr<EntityPrivate, Delector> node_root;

		std::vector<std::unique_ptr<Closure<void(Capture&, System*, bool)>>> update_listeners;

		void release() override { delete this; }

		void register_object(void* o, const std::string& name);
		void register_object(void* o, const char* name) override { register_object(o, std::string(name)); }
		void* find_object(const std::string& name) const;
		void* find_object(const char* name) const override { return find_object(std::string(name)); }

		System* get_system(uint type_hash) const override;
		System* find_system(const std::string& name) const;
		System* find_system(const char* name) const override { return find_system(std::string(name)); }
		void add_system(System* s) override;
		void remove_system(System* s) override;

		Entity* get_element_root() override;
		Entity* get_node_root() override;

		void update() override;

		void* add_update_listener(void (*callback)(Capture& c, System* system, bool before), const Capture& capture) override;
		void remove_update_listener(void* ret) override;
	};
}
