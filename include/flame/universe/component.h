#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		const char* name;
		const uint name_hash;

		Entity* entity;

		Listeners<void(void* c, uint hash)> data_changed_listeners;

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		virtual void on_added() {} // on this component added to entity; or on this component's entity added to parent
		virtual void on_into_world() {} // on this component's entity got world_
		virtual void on_component_added(Component* c) {} // on new component added to this component's entity; or on this component added to entity, to tell this component other components on the entity
		virtual void on_child_component_added(Component* c) {} // same thing happened on child
		virtual void on_component_removed(Component* c) {} // on other components removed from this components's entity
		virtual void on_child_component_removed(Component* c) {} // same thing happened on child
		virtual void on_visibility_changed() {} // on this component's visibility changed
		virtual void on_child_visibility_changed() {} // same thing happened on child
		virtual void on_position_changed() {} // on this component's postion changed
		virtual void on_child_position_changed(Entity* e) {} // same thing happened on child
		virtual Component* copy() { return nullptr; }
	};

	// component type may has a type for serialization
	// the type name is 'Component' + component type name + '$'
	// the type contains the data for serialization
}
