#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct sUIRenderer : System
	{
		sUIRenderer() :
			System("sUIRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sUIRenderer* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(sUIRenderer* s);
	};
}
