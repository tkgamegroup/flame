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
		inline PxVec3 cvt(const Vec3f& v)
		{
			return PxVec3(v.x(), v.y(), v.z());
		}

		inline Vec3f cvt(const PxVec3& v)
		{
			return Vec3f(v.x, v.y, v.z);
		}

		inline PxQuat cvt(const Vec4f& v)
		{
			return PxQuat(v.x(), v.y(), v.z(), v.w());
		}

		inline Vec4f cvt(const PxQuat& v)
		{
			return Vec4f(v.x, v.y, v.z, v.w);
		}
	}
}

