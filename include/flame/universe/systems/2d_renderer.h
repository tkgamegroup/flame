#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct s2DRenderer : System
	{
		s2DRenderer() :
			System("s2DRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static s2DRenderer* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(s2DRenderer* s);
	};
}
