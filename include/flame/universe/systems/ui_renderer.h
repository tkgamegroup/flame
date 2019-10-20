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

		FLAME_UNIVERSE_EXPORTS static sUIRenderer* create(graphics::Canvas* canvas);
	};
}
