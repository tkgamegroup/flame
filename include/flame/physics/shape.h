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

