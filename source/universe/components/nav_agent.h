#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cNavAgent : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		/// Reflect
		float radius = 0.2f;
		/// Reflect
		float height = 2.f;
		/// Reflect
		float speed = 5.f;
		/// Reflect
		float speed_scale = 1.f;
		/// Reflect
		virtual void set_speed_scale(float v) = 0;
		/// Reflect
		float turn_speed = 900.f;
		/// Reflect
		float turn_speed_scale = 1.f;
		/// Reflect
		virtual void set_turn_speed_scale(float v) = 0;
		/// Reflect
		uint separation_group = 1;

		virtual void set_target(const vec3& pos, bool face_mode = false) = 0;
		virtual void stop() = 0;
		virtual vec3 desire_velocity() = 0;
		virtual vec3 current_velocity() = 0;

		struct Create
		{
			virtual cNavAgentPtr operator()(EntityPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
