#pragma once

#include <flame/physics/rigid.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct ShapePrivate;

		struct RigidBridge : Rigid
		{
			void add_shape(Shape* s) override;
			void remove_shape(Shape* s) override;
		};

		struct RigidPrivate : RigidBridge
		{
			bool dynamic;

#ifdef USE_PHYSX
			PxRigidActor* px_rigid;
#endif

			RigidPrivate(bool dynamic);
			~RigidPrivate();

			void release() override { delete this; }

			void get_pose(Vec3f& coord, Vec4f& quat) const override;
			void set_pose(const Vec3f& coord, const Vec4f& quat) override;

			void add_shape(ShapePrivate* s);
			void remove_shape(ShapePrivate* s);

			void add_impulse(const Vec3f& v) override;
			void add_force(const Vec3f& v) override;
			void clear_force() override;
		};

		inline void RigidBridge::add_shape(Shape* s)
		{
			((RigidPrivate*)this)->add_shape((ShapePrivate*)s);
		}

		inline void RigidBridge::remove_shape(Shape* s)
		{
			((RigidPrivate*)this)->remove_shape((ShapePrivate*)s);
		}
	}
}

