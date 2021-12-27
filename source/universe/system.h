#pragma once

#include "universe.h"

namespace flame
{
	struct System
	{
		uint type_hash = 0;

		WorldPtr world = nullptr;

		virtual ~System() {}

		virtual void update() = 0;
	};
}
