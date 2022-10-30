#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cNavScene : Component
	{
		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		float agent_radius = 0.6f;
		// Reflect
		float agent_height = 1.8f;
		// Reflect
		float walkable_climb = 0.5f;
		// Reflect
		float walkable_slope_angle = 45.f;

		struct Create
		{
			virtual cNavScenePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
