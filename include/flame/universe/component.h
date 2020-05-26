#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Component : Object
	{
		Entity* entity;

		void* user_data;

		Array<Component*> sibling_data_changed_info_components;
		Array<Component*> parent_data_changed_info_components;
		ListenerHub<bool(Capture& c, uint hash, void* sender)> data_changed_listeners;
		FLAME_UNIVERSE_EXPORTS virtual void on_event(Entity::Event e, void* t) {}
		FLAME_UNIVERSE_EXPORTS virtual void on_sibling_data_changed(Component* t, uint hash, void* sender) {}
		FLAME_UNIVERSE_EXPORTS virtual void on_child_data_changed(Component* t, uint hash, void* sender) {}

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		FLAME_UNIVERSE_EXPORTS void data_changed(uint hash, void* sender);
	};
}
