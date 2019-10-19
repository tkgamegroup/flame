#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	struct System
	{
		virtual ~System() {};
		virtual void update(Entity* root);
	};
}
