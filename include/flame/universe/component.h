#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Component : Object
	{
		Entity* entity;

		void* user_data;

		ListenerHub<bool(Capture& c, uint hash, void* sender)> data_changed_listeners;
		virtual void on_event(EntityEvent e, void* t) {}
		virtual void on_sibling_data_changed(Component* t, uint hash, void* sender) {}
		virtual void on_child_data_changed(Component* t, uint hash, void* sender) {}

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		FLAME_UNIVERSE_EXPORTS void data_changed(uint hash, void* sender);
	};
}
