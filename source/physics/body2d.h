#pragma once

#include "physics.h"

namespace flame
{
	namespace physics
	{
		struct Shape2d
		{
			ShapeType type;
			union 
			{
				struct Box
				{
					vec2 center;
					vec2 hf_ext;
				}box;
				struct Circle
				{
					vec2 center;
					float radius;
				}circle;
			}data;

			inline void as_box(const vec2& center, const vec2& hf_ext)
			{
				type = ShapeBox;
				data.box.center = center;
				data.box.hf_ext = hf_ext;
			}

			inline void as_circle(const vec2& center, float radius)
			{
				type = ShapeCircle;
				data.circle.center = center;
				data.circle.radius = radius;
			}
		};

		struct Body2d
		{
			vec2 pos;
			float angle;

			voidptr user_data = nullptr;

			virtual ~Body2d() {}

			virtual void add_shape(const Shape2d& shape, float density = 1.f, float friction = 0.5f, ushort collide_bit = 1, ushort collide_mask = 0xffff) = 0;

			virtual float get_mass() = 0;
			virtual vec2 get_velocity() = 0;
			virtual void set_velocity(const vec2& vel) = 0;
			virtual void set_pos(const vec2& pos) = 0;
			virtual void apply_force(const vec2& force) = 0;

			struct Create
			{
				virtual Body2dPtr operator()(World2dPtr world, BodyType type, const vec2& pos) = 0;
			};
			FLAME_PHYSICS_API static Create& create;
		};
	}
}
