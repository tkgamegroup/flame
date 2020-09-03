#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct sPhysicsWorld : System
	{
		inline static auto type_name = "flame::sPhysicsWorld";
		inline static auto type_hash = ch(type_name);

		sPhysicsWorld() :
			System(type_name, type_hash)
		{
		}

		FLAME_UNIVERSE_EXPORTS static sPhysicsWorld* create();
	};
}
