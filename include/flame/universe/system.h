#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct World;
	struct WorldPrivate;

	struct System
	{
		const char* type_name;
		const uint64 type_hash;

#ifdef FLAME_UNIVERSE_MODULE
		WorldPrivate* world = nullptr;
#else
		World* world = nullptr;
#endif

		System(const char* name, uint64 hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~System() {}

		virtual void on_added() {}
		virtual void on_removed() {}
		virtual void update() = 0;
	};
}
