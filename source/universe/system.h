#pragma once

#include "universe.h"

namespace flame
{
	struct System
	{
		const char* type_name;
		const uint type_hash;

		WorldPtr world = nullptr;

		System(const char* name, uint hash) :
			type_name(name),
			type_hash(hash)
		{
		}

		virtual ~System() {}

		virtual void on_added() {}
		virtual void on_removed() {}
		virtual void update() = 0;
	};
}
