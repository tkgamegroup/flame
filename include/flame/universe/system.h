#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;
	struct World;

	struct System : Object
	{
		World* world = nullptr;

		System(const char* name) :
			Object(name)
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
