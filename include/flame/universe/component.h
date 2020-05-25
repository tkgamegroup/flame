#pragma once

#include <flame/universe/entity.h>

namespace flame
{
	struct Component : Object
	{
		Entity* entity;

		void* user_data;

		ListenerHub<bool(Capture& c, uint hash, void* sender)> data_changed_listeners;

		FLAME_UNIVERSE_EXPORTS Component(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~Component();

		FLAME_UNIVERSE_EXPORTS void data_changed(uint hash, void* sender);

		virtual void on_added() {}
	};
}
