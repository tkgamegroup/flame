#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct Component
	{
		const char* type_name;
		const uint64 type_hash;

		Entity* entity = nullptr;

		Component(const char* name, uint64 hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~Component() {}

		// this component added to entity
		virtual void on_added() {}

		// this component removed from entity
		virtual void on_removed() {}

		// local event, this component's entity entered world or this component added to a entity that entered world
		virtual void on_entered_world() {}

		// local event, this component's entity left world or this component removed from a entity that entered world
		virtual void on_left_world() {}

		// local event, this component's entity destroyed
		virtual void on_entity_destroyed() {}

		// local event, this component's entity visibility changed
		virtual void on_entity_visibility_changed() {}

		// local event, this component's entity state changed
		virtual void on_entity_state_changed() {}

		// local event, this component's entity added to another entity
		virtual void on_entity_added() {}

		// local event, this component's entity removed from parent entity
		virtual void on_entity_removed() {}

		// local event, this component's entity's position changed
		virtual void on_entity_position_changed() {}

		// local event, this component's entity added a component
		virtual void on_entity_component_added(Component* c) {}

		// local event, this component's entity removed a component
		virtual void on_entity_component_removed(Component* c) {}

		// local event, this component's entity added another entity
		virtual void on_entity_added_child(Entity* e) {}

		// local event, this component's entity removed an entity
		virtual void on_entity_removed_child(Entity* e) {}

		// child event, this component's entity's child visibility changed
		virtual void on_entity_child_visibility_changed(Entity* e) {}

		// child event, this component's entity's child's position changed
		virtual void on_entity_child_position_changed(Entity* e) {}

		// child event, this component's entity's child added a component
		virtual void on_entity_child_component_added(Component* c) {}

		// child event, this component's entity's child removed a component
		virtual void on_entity_child_component_removed(Component* c) {}

		// local data changed, other component on this component's entity data changed
		virtual void on_entity_component_data_changed(Component* c, uint data_name_hash) {}

		// child data changed, component on this component's entity's child data changed
		virtual void on_entity_child_component_data_changed(Component* c, uint data_name_hash) {}
	};
}
