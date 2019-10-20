#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		const char* type_name;
		const uint type_hash;

		Entity* entity;

		Listeners<void(void* c, uint hash)> data_changed_listeners;

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		virtual void on_added() {}
		virtual void on_component_added(Component* c) {}
		virtual void on_child_component_added(Component* c) {}
		virtual void on_component_removed(Component* c) {}
		virtual void on_child_component_removed(Component* c) {}
		virtual void on_visibility_changed() {}
		virtual void on_child_visibility_changed() {}
		virtual void on_position_changed() {}
		virtual void on_child_position_changed(Entity* e) {}
		virtual Component* copy() { return nullptr; }
	};

	// component type may has a type for serialization
	// the type name is 'Component' + component type name + '$'
	// the type contains the data for serialization
}
