#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct System
	{
		const char* name;
		const uint name_hash;

		System(const char* name) :
			name(name),
			name_hash(H(name))
		{
		}

		virtual ~System() {};
		virtual void update(Entity* root) = 0;
	};
}
