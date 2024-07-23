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

			void add_shape(const Shape2d& shape, float density = 1.f, float friction = 0.5f, ushort collide_bit = 1, ushort collide_mask = 0xffff) override;

			float get_mass() override;
			vec2 get_velocity() override;
			void set_velocity(const vec2& vel) override;
			void set_pos(const vec2& pos) override;
			void apply_force(const vec2& force) override;
		};
	}
}
