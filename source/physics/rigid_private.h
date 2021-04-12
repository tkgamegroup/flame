#pragma once

#include "rigid.h"
#include "physics_private.h"

namespace flame
{
	namespace physics
	{
		struct RigidPrivate : Rigid
		{
			bool dynamic;

#ifdef USE_PHYSX
			UniPtr<PxRigidActor> px_rigid;
#endif

			RigidPrivate(DevicePrivate* device, bool dynamic);

			void release() override { delete this; }

			void get_pose(vec3& coord, quat& qut) const override;
			void set_pose(const vec3& coord, const quat& qut) override;

			void add_shape(ShapePtr s) override;
			void remove_shape(ShapePtr s) override;

			void add_impulse(const vec3& v) override;
			void add_force(const vec3& v) override;
			void clear_force() override;
		};
	}
}

