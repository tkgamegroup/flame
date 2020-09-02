#pragma once

#include <flame/physics/rigid.h>
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct DevicePrivate;
		struct ShapePrivate;

		struct RigidBridge : Rigid
		{
			void add_shape(Shape* s) override;
			void remove_shape(Shape* s) override;
		};

		struct RigidPrivate : RigidBridge
		{
#ifdef USE_PHYSX
			physx::PxRigidActor* px_rigid;
#endif

			RigidPrivate(DevicePrivate* d, const Vec3f& coord, bool dynamic);
			~RigidPrivate();

			void release() override { delete this; }

			void add_shape(ShapePrivate* s);
			void remove_shape(ShapePrivate* s);
			void get_pose(Vec3f& out_coord, Vec4f& out_quat) override;
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

