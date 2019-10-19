#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string name;
		uint name_hash;

		std::unordered_map<uint, std::unique_ptr<Component>> components;
		EntityPrivate* parent;
		std::vector<std::unique_ptr<EntityPrivate>> children;

		EntityPrivate();
		void set_visibility(bool v);
		Component* get_component(uint type_hash);
		Mail<std::vector<Component*>> get_components();
		void add_component(Component* c);
		void remove_component(Component* c);
		EntityPrivate* find_child(const std::string& name) const;
		void add_child(EntityPrivate* e, int position);
		void reposition_child(EntityPrivate* e, int position);
		void mark_dying();
		void remove_child(EntityPrivate* e, bool destroy);
		EntityPrivate* copy();
		void update_visibility();
	};
}
