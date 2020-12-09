#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct EntityPrivate;
	struct Component;

	struct Driver
	{
		const char* type_name;

#ifdef FLAME_UNIVERSE_MODULE
		EntityPrivate* entity = nullptr;
#else
		Entity* entity = nullptr;
#endif

		Driver(const char* name) :
			type_name(name)
		{
		}

		virtual bool on_read_property(Component* c, const char* value) { return true; }
		virtual void on_load_finished() {}
	};
}
