#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct World;

	struct System : Object
	{
		World* world_;

		ListenerHub<bool(Capture& c)> before_update_listeners;
		ListenerHub<bool(Capture& c)> after_update_listeners;

		FLAME_UNIVERSE_EXPORTS System(const char* name);
		FLAME_UNIVERSE_EXPORTS virtual ~System();

		virtual void on_added() {}
		virtual void before_update() {}
		virtual void update() = 0;
		virtual void after_update() {}
	};
}
