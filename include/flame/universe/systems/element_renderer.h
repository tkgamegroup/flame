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
		sElementRenderer() :
			System("sElementRenderer")
		{
		}

		virtual void release() = 0;

		FLAME_UNIVERSE_EXPORTS static sElementRenderer* create();
	};
}
