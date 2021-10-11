#pragma once

#include "physics.h"

#ifdef USE_PHYSX
#include <PxPhysicsAPI.h>
#endif

namespace flame
{
	namespace physics
	{
#ifdef USE_PHYSX
		inline physx::PxVec3 cvt(const vec3& v)
		{
			return physx::PxVec3(v.x, v.y, v.z);
		}

		inline vec3 cvt(const physx::PxVec3& v)
		{
			return vec3(v.x, v.y, v.z);
		}

		inline physx::PxQuat cvt(const quat& v)
		{
			return physx::PxQuat(v.x, v.y, v.z, v.w);
		}

		inline quat cvt(const physx::PxQuat& v)
		{
			return quat(v.w, v.x, v.y, v.z);
		}
#endif
	}
}

