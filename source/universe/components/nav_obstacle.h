#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cNavObstacle : Component
	{
		enum Type
		{
			TypeCylinder,
			TypeBox
		};

		// Reflect requires
		cNodePtr node = nullptr;

		// Reflect
		Type type = TypeCylinder;
		// Reflect
		vec3 extent = vec3(1.f); // for cylinder: radius, height; for box: half extents

		struct Create
		{
			virtual cNavObstaclePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
