#pragma once

#include "universe.h"

namespace flame
{
	struct System
	{
		uint type_hash = 0;
		WorldPtr world = nullptr;
		bool enable = true;

		uint n_strong_ref = 0;
		uint update_times = 0;

		virtual ~System() {}

		virtual void start() {}
		virtual void update() = 0;
	};
}
