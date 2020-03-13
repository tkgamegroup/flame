#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct System : Object
	{
		World* world_;

		ListenerHub<bool(void* c)> before_update_listeners;
		ListenerHub<bool(void* c)> after_update_listeners;

		System(const char* name) :
			Object(name),
			world_(nullptr)
		{
		}

		virtual ~System() {};
		virtual void on_added() {}
		virtual void update() = 0;
	};
}
