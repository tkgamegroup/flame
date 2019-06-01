//MIT License
//
//Copyright (c) 2018 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#pragma once

#pragma once

#include "physics.h"

#include <PxPhysicsAPI.h>
using namespace physx;

namespace flame
{
	namespace physics
	{
		inline PxVec3 Z(const Vec3 &v)
		{
			return PxVec3(v.x(), v.y(), v.z());
		}

		inline Vec3 Z(const PxVec3 &v)
		{
			return Vec3(v.x(), v.y(), v.z());
		}

		inline PxMat33 Z(const Mat3 &m)
		{
			return PxMat33(
				PxVec3(m[0][0], m[0][1], m[0][2]),
				PxVec3(m[1][0], m[1][1], m[1][2]),
				PxVec3(m[2][0], m[2][1], m[2][2])
			);
		}

		inline PxTransform Z(const Vec3 &coord, const Vec4 &quat)
		{
			return PxTransform(Z(coord), PxQuat(quat.x(), quat.y(), quat.z(), quat.w()));
		}

		inline PxTransform Z(const Vec3 &coord, const Mat3 &axis)
		{
			return PxTransform(Z(coord), PxQuat(Z(axis)));
		}
	}
}

