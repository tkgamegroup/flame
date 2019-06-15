#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Shape;

		struct RigidPrivate;

		struct Rigid
		{
			RigidPrivate *_priv;

			FLAME_PHYSICS_EXPORTS void attach_shape(Shape *s);
			FLAME_PHYSICS_EXPORTS void detach_shape(Shape *s);
			FLAME_PHYSICS_EXPORTS void get_pose(Vec3f &out_coord, Vec4f &out_quat);
			FLAME_PHYSICS_EXPORTS void add_force(const Vec3f &v);
			FLAME_PHYSICS_EXPORTS void clear_force();
		};

		FLAME_PHYSICS_EXPORTS Rigid *create_static_rigid(Device *d, const Vec3f &coord);
		FLAME_PHYSICS_EXPORTS Rigid *create_dynamic_rigid(Device *d, const Vec3f &coord);
		FLAME_PHYSICS_EXPORTS void destroy_rigid(Rigid *r);
	}
}

