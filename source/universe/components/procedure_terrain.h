#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cProcedureTerrain : Component
	{
		// Reflect requires
		cTerrainPtr terrain = nullptr;
	};
}
