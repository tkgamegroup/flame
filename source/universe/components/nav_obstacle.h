#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cNavObstacle : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		struct Create
		{
			virtual cNavObstaclePtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
