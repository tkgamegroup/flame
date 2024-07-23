#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Body2d
		{
			vec2 pos;
			float angle;

			voidptr user_data = nullptr;

			virtual ~Body2d() {}

			virtual float get_mass() = 0;
			virtual vec2 get_velocity() = 0;
			virtual void set_pos(const vec2& pos) = 0;
			virtual void apply_force(const vec2& force) = 0;

			struct CreateBox
			{
				virtual Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos, const vec2& hf_ext, float density = 1.f, float friction = 0.5f) = 0;
			};
			FLAME_PHYSICS_API static CreateBox& create_box;

			struct CreateCircle
			{
				virtual Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos, float radius, float density = 1.f, float friction = 0.5f) = 0;
			};
			FLAME_PHYSICS_API static CreateCircle& create_circle;
		};
	}
}
