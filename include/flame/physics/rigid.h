#pragma once

#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Shape;

		struct Rigid
		{
			virtual void release() = 0;

			virtual void get_pose(Vec3f& coord, Vec4f& quat) const = 0;
			virtual void set_pose(const Vec3f& coord, const Vec4f& quat) = 0;

			virtual void add_shape(Shape* s) = 0;
			virtual void remove_shape(Shape* s) = 0;

			virtual void add_impulse(const Vec3f& v) = 0;
			virtual void add_force(const Vec3f& v) = 0;
			virtual void clear_force() = 0;

			void* user_data;

			FLAME_PHYSICS_EXPORTS static Rigid* create(bool dynamic);
		};
	}
}

