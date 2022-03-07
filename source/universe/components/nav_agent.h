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
		float height = 2.f;
		/// Reflect
		float speed = 3.5f;
		/// Reflect
		float turn_speed = 360.f;

		virtual void set_target(const vec3& pos) = 0;
		virtual void stop() = 0;

		struct Create
		{
			virtual cNavAgentPtr operator()(EntityPtr) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
