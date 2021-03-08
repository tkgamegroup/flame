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
			bool dynamic;

#ifdef USE_PHYSX
			FlmPtr<PxRigidActor> px_rigid;
#endif

			RigidPrivate(DevicePrivate* device, bool dynamic);

			void release() override { delete this; }

			void get_pose(vec3& coord, quat& qut) const override;
			void set_pose(const vec3& coord, const quat& qut) override;

			void add_shape(ShapePrivate* s);
			void remove_shape(ShapePrivate* s);

			void add_impulse(const vec3& v) override;
			void add_force(const vec3& v) override;
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

