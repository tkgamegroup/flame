#pragma once

#include <flame/physics/physics.h>

#ifdef USE_PHYSX
#include <PxPhysicsAPI.h>
#endif

using namespace physx;

namespace flame
{
	namespace physics
	{
		inline PxVec3 cvt(const vec3& v)
		{
			return PxVec3(v.x, v.y, v.z);
		}

		inline vec3 cvt(const PxVec3& v)
		{
			return vec3(v.x, v.y, v.z);
		}

		inline PxQuat cvt(const vec4& v)
		{
			return PxQuat(v.x, v.y, v.z, v.w);
		}

		inline vec4 cvt(const PxQuat& v)
		{
			return vec4(v.x, v.y, v.z, v.w);
		}
	}
}

