#pragma once

#include <flame/universe/system.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct sUIRenderer : System
	{
		sUIRenderer() :
			System("UIRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sUIRenderer* create(graphics::Canvas* canvas);
		FLAME_UNIVERSE_EXPORTS static void destroy(sUIRenderer* s);
	};
}
