#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Component : Object
	{
		Entity* entity;

		virtual void on_event(EntityEvent e, void* t) {}
		virtual void on_local_data_changed(Component* t, uint hash, void* sender) {} // other components on this entity
		virtual void on_child_data_changed(Component* t, uint hash, void* sender) {} // components on all children of this entity

		FLAME_UNIVERSE_EXPORTS Component(const char* name, Entity* e);
		virtual ~Component() {}

		FLAME_UNIVERSE_EXPORTS void data_changed(uint hash, void* sender);
	};
}
