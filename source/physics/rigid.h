#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Rigid
		{
			virtual void release() = 0;

			virtual bool is_sleeping() const = 0;

			virtual void get_pose(vec3& coord, quat& qut) const = 0;
			virtual void set_pose(const vec3& coord, const quat& qut) = 0;

			virtual void add_shape(ShapePtr s) = 0;
			virtual void remove_shape(ShapePtr s) = 0;

			virtual void add_impulse(const vec3& v) = 0;
			virtual void add_force(const vec3& v) = 0;
			virtual void clear_force() = 0;

			void* user_data;

			FLAME_PHYSICS_EXPORTS static Rigid* create(Device* device, bool dynamic);
		};
	}
}

