#pragma once

#include "universe.h"

namespace flame
{
	struct System
	{
		const uint type_hash;

		WorldPtr world = nullptr;

		virtual ~System() {}

		virtual void update() = 0;
	};
}
