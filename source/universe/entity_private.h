#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct EntityPrivate : Entity
	{
		std::string name;
		uint name_hash;
		std::vector<std::unique_ptr<Component>> components;
		EntityPrivate* parent;
		std::vector<std::unique_ptr<EntityPrivate>> children;

		EntityPrivate();
		Component* find_component(uint type_hash);
		Mail<std::vector<Component*>> find_components(uint type_hash);
		void add_component(Component* c);
		void remove_component(Component* c);
		EntityPrivate* find_child(const std::string& name) const;
		void add_child(EntityPrivate* e, int position);
		void reposition_child(EntityPrivate* e, int position);
		void mark_dying();
		void remove_child(EntityPrivate* e, bool destroy);
		void remove_all_children(bool destroy);
		EntityPrivate* copy();
		void traverse_forward(void (*callback)(void* c, Entity* n), const Mail<>& capture);
		void traverse_backward(void (*callback)(void* c, Entity* n), const Mail<>& capture);
		void update();
	};

	struct ListenerHub
	{
		std::vector<std::unique_ptr<Closure<void(void* c)>>> listeners;
	};
}
