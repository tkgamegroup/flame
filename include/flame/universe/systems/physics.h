#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct sPhysics : System
	{
		inline static auto type_name = "flame::sPhysics";
		inline static auto type_hash = ch(type_name);

		sPhysics() :
			System(type_name, type_hash)
		{
		}

		virtual vec3 raycast(const vec3& origin, const vec3& dir) = 0;
		virtual void set_visualization(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static sPhysics* create(void* parms = nullptr);
	};
}
