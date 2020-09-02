#pragma once

#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Shape;

		struct Rigid
		{
			virtual void release() = 0;

			virtual void add_shape(Shape* s) = 0;
			virtual void remove_shape(Shape* s) = 0;
			virtual void get_pose(Vec3f& out_coord, Vec4f& out_quat) = 0;
			virtual void add_force(const Vec3f& v) = 0;
			virtual void clear_force() = 0;

			FLAME_PHYSICS_EXPORTS static Rigid* create(Device* d, const Vec3f& coord, bool dynamic);
		};
	}
}

