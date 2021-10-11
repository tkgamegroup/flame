#pragma once

#include "physics.h"

#ifdef USE_PHYSX
#include <PxPhysicsAPI.h>
using namespace physx;
#endif

namespace flame
{
	namespace physics
	{
#ifdef USE_PHYSX
		inline PxVec3 cvt(const vec3& v)
		{
			return PxVec3(v.x, v.y, v.z);
		}

		inline vec3 cvt(const PxVec3& v)
		{
			return vec3(v.x, v.y, v.z);
		}

		inline PxQuat cvt(const quat& v)
		{
			return PxQuat(v.x, v.y, v.z, v.w);
		}

		inline quat cvt(const PxQuat& v)
		{
			return quat(v.w, v.x, v.y, v.z);
		}
#endif
	}
}

