#pragma once
#ifdef USE_PHYSICS_MODULE

#include "../system.h"

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

		virtual vec3 raycast(const vec3& origin, const vec3& dir, EntityPtr* out_e = nullptr) = 0;
		virtual void set_visualization(bool v) = 0;

		FLAME_UNIVERSE_EXPORTS static sPhysics* create(void* parms = nullptr);
	};
}

#endif
