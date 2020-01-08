#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/entity.h>

namespace flame
{
	struct Entity;

	struct Component : Object
	{
		Entity* entity;

		ListenerHub<void(void* c, Component* thiz, uint hash, void* sender)> data_changed_listeners;

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		FLAME_UNIVERSE_EXPORTS void data_changed(uint hash, void* sender);

		virtual void on_added() {} // on this component added to entity; or on this component's entity added to parent
		virtual void on_entered_world() {} // on this component's entity entered world
		virtual void on_left_world() {} // on this component's entity left world
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
}
