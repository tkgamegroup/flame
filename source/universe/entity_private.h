#pragma once

#include <flame/universe/entity.h>
#include <flame/universe/component.h>

namespace flame
{
	struct WorldPrivate;

	struct EntityPrivate : Entity
	{
		std::string name;
		uint name_hash;

		WorldPrivate* world;
		EntityPrivate* parent;
		std::unordered_map<uint, std::unique_ptr<Component>> components;
		std::vector<std::unique_ptr<EntityPrivate>> children;

		EntityPrivate();
		~EntityPrivate();
		void set_visibility(bool v);
		Component* get_component_plain(uint name_hash);
		Array<Component*> get_components();
		void add_component(Component* c);
		void remove_component(Component* c);
		EntityPrivate* find_child(const std::string& name) const;
		int find_child(EntityPrivate* e) const;
		void add_child(EntityPrivate* e, int position);
		void reposition_child(EntityPrivate* e, int position);
		void mark_dying();
		void remove_child(EntityPrivate* e, bool destroy);
		void remove_children(int from, int to, bool destroy);
		EntityPrivate* copy();
		void update_visibility();
	};
}
