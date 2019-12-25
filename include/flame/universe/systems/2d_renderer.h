#pragma once

#include <flame/universe/system.h>

namespace flame
{
	namespace graphics
	{
		struct SwapchainResizable;
		struct Commandbuffer;
		struct Canvas;
	}

	struct s2DRenderer : System
	{
		graphics::Canvas* canvas;

		s2DRenderer() :
			System("s2DRenderer")
		{
		}

		FLAME_UNIVERSE_EXPORTS static s2DRenderer* create(const std::wstring& canvas_filename, void* dst, uint dst_hash, void* cbs);
		FLAME_UNIVERSE_EXPORTS static void destroy(s2DRenderer* s);
	};
}
