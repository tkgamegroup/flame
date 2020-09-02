#pragma once

#pragma once

#include <flame/physics/physics.h>

#ifdef USE_PHYSX
#include <PxPhysicsAPI.h>
#endif

namespace flame
{
	namespace physics
	{
#ifdef USE_PHYSX
		inline PxVec3 Z(const Vec3f &v)
		{
			return PxVec3(v.x(), v.y(), v.z());
		}

		inline Vec3f Z(const PxVec3 &v)
		{
			return Vec3f(v.x(), v.y(), v.z());
		}

		inline PxMat33 Z(const Mat3 &m)
		{
			return PxMat33(
				PxVec3(m[0][0], m[0][1], m[0][2]),
				PxVec3(m[1][0], m[1][1], m[1][2]),
				PxVec3(m[2][0], m[2][1], m[2][2])
			);
		}

		inline PxTransform Z(const Vec3f &coord, const Vec4f &quat)
		{
			return PxTransform(Z(coord), PxQuat(quat.x(), quat.y(), quat.z(), quat.w()));
		}

		inline PxTransform Z(const Vec3f &coord, const Mat3 &axis)
		{
			return PxTransform(Z(coord), PxQuat(Z(axis)));
		}
#endif
	}
}

