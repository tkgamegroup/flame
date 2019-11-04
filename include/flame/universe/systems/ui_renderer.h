#pragma once

#include <flame/universe/system.h>

namespace flame
{
	struct sUIRenderer : System
	{
		sUIRenderer() :
			System("UIRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sUIRenderer* create();
		FLAME_UNIVERSE_EXPORTS static void destroy(sUIRenderer* s);
	};
}
