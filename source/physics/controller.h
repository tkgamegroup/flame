#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Controller
		{
			virtual void release() = 0;

			virtual vec3 get_position() const = 0;
			virtual void set_position(const vec3& pos) = 0;
			virtual bool move(const vec3& disp, float delta_time) = 0;

			void* user_data;

			FLAME_PHYSICS_EXPORTS static Controller* create(Scene* scene, Material* material, float radius, float height);
		};
	}
}
