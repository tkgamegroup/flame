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

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Device;
		struct Shape;

		struct RigidPrivate;

		struct Rigid
		{
			RigidPrivate *_priv;

			FLAME_PHYSICS_EXPORTS void attach_shape(Shape *s);
			FLAME_PHYSICS_EXPORTS void detach_shape(Shape *s);
			FLAME_PHYSICS_EXPORTS void get_pose(Vec3 &out_coord, Vec4 &out_quat);
			FLAME_PHYSICS_EXPORTS void add_force(const Vec3 &v);
			FLAME_PHYSICS_EXPORTS void clear_force();
		};

		FLAME_PHYSICS_EXPORTS Rigid *create_static_rigid(Device *d, const Vec3 &coord);
		FLAME_PHYSICS_EXPORTS Rigid *create_dynamic_rigid(Device *d, const Vec3 &coord);
		FLAME_PHYSICS_EXPORTS void destroy_rigid(Rigid *r);
	}
}

