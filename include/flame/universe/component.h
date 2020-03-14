#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Entity;

	struct Component : Object
	{
		Entity* entity;

		ListenerHub<bool(void* c, uint hash, void* sender)> data_changed_listeners;

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		void data_changed(uint hash, void* sender)
		{
			if (sender != INVALID_POINTER)
				data_changed_listeners.call(hash, sender);
		}

		virtual void on_added() {}
		virtual void on_entered_world() {}
		virtual void on_left_world() {}
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
}
