#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct System : Object
	{
		System(const char* name) :
			Object(name)
		{
		}

		virtual ~System() {};
		virtual void update(Entity* root) = 0;
	};
}
