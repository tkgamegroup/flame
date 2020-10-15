#pragma once

#include <flame/physics/physics.h>

namespace flame
{
	namespace physics
	{
		struct Scene;

		struct Controller
		{
			virtual void release() = 0;

			virtual Vec3f get_position() const = 0;
			virtual void set_position(const Vec3f& pos) = 0;
			virtual void move(const Vec3f& disp, float delta_time) = 0;

			FLAME_PHYSICS_EXPORTS static Controller* create(Scene* scene, float radius, float height);
		};
	}
}
