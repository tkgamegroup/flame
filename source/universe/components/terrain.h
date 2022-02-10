#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cTerrain : Component
	{
		/// Reflect requires
		cNodePtr node = nullptr;

		struct Create
		{
			virtual cTerrainPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
