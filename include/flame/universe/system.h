#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct World;

	struct System
	{
		const char* type_name;
		const uint64 type_hash;

		World* world = nullptr;

		System(const char* name, uint64 hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~System() {}

		// this system added to world
		virtual void on_added() {}

		// this system removed to world
		virtual void on_removed() {}

		virtual void update() = 0;
	};
}
