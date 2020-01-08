#pragma once

#include <flame/foundation/foundation.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct System : Object
	{
		World* world_;

		System(const char* name) :
			Object(name),
			world_(nullptr)
		{
		}

		virtual ~System() {};
		virtual void on_added() {}
		virtual void update(Entity* root) = 0;
	};
}
