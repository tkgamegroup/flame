#pragma once

#include "universe.h"

namespace flame
{
	struct System
	{
		uint type_hash = 0;
		WorldPtr world = nullptr;
		bool enable = true;

		ushort n_ref = 0;
		ushort update_times = 0;

		virtual ~System() {}

		virtual void start() {}
		virtual void update() = 0;
	};
}
