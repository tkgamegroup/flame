#pragma once

#include "physics_private.h"
#include "world2d_private.h"
#include "body2d.h"

namespace flame
{
	namespace physics
	{
		struct Body2dPrivate : Body2d
		{
			b2Body* b2_body = nullptr;
			World2dPtr world = nullptr;

			Body2dPrivate(World2dPtr world);
			~Body2dPrivate();

			float get_mass() override;
			vec2 get_velocity() override;
			void set_pos(const vec2& pos) override;
			void apply_force(const vec2& force) override;
		};
	}
}
