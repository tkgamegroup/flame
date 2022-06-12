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
		float radius = 0.3f;
		/// Reflect
		float height = 1.8f;
		/// Reflect
		float speed = 7.5f;
		/// Reflect
		float turn_speed = 900.f;

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
