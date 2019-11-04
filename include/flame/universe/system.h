#pragma once

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

	// system type may has a type for serialization
	// the type name is 'System' + system type name + '$'
	// the type contains the data for serialization
}
