#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct EntityPrivate;

	struct Driver
	{
		const char* type_name;
		const uint type_hash;

#ifdef FLAME_UNIVERSE_MODULE
		EntityPrivate* entity = nullptr;
#else
		Entity* entity = nullptr;
#endif

		int src_id = -1;

		bool load_finished = false;

		Driver(const char* name, uint hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual void on_load_finished() {}
		virtual bool on_child_added(Entity* e) { return false; }
	};
}
