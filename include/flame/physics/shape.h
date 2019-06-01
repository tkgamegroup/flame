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
		struct Material;

		struct ShapePrivate;

		struct Shape
		{
			ShapePrivate *_priv;

			FLAME_PHYSICS_EXPORTS void set_trigger(bool v); // default false

			/*  == set_trigger ==
				A trigger means it will not collide with others, but will report when it overlay
				with others. We can use this for items, speical area etc.
			*/
		};

		//FLAME_PHYSICS_EXPORTS void create_sphere_shape(Material *m, const glm::vec3 &coord,
		//	float radius);
		//FLAME_PHYSICS_EXPORTS void create_capsule_shape(Material *m, const glm::vec3 &coord,
		//	float radius, float height);

		FLAME_PHYSICS_EXPORTS Shape *create_box_shape(Device *d, Material *m, const Vec3f &coord,
			float x_hf_ext, float y_hf_ext, float z_hf_ext);
		FLAME_PHYSICS_EXPORTS void destroy_shape(Shape *s);
	}
}

