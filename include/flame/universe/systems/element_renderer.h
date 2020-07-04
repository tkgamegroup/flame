#pragma once

#include <flame/universe/system.h>

namespace flame
{
	namespace graphics
	{
		struct Commandbuffer;
		struct Canvas;
	}

	struct sElementRenderer : System
	{
		graphics::Canvas* canvas;

		bool pending_update;

		sElementRenderer() :
			System("sElementRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static sElementRenderer* create(graphics::Canvas* canvas);
		FLAME_UNIVERSE_EXPORTS static void destroy(sElementRenderer* s);
	};
}
