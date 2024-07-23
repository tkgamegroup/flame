#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cBody2d : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		physics::BodyType type = physics::BodyDynamic;
		// Reflect
		physics::ShapeType shape_type = physics::ShapeBox;
		// Reflect
		vec2 hf_ext = vec2(0.5f);
		// Reflect
		float radius = 0.5f;
		// Reflect
		float density = 1.f;
		// Reflect
		float friction = 0.5f;
		// Reflect
		ushort collide_bit = 1;
		// Reflect
		ushort collide_mask = 0xffff;

		float mass = 0.f;

		virtual vec2 get_velocity() = 0;
		virtual void set_velocity(const vec2& vel) = 0;
		virtual void upload_pos() = 0;
		virtual void apply_force(const vec2& force) = 0;

		struct Create
		{
			virtual cBody2dPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
