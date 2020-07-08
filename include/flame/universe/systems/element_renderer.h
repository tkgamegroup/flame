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
		inline static auto type_name = "sElementRenderer";
		inline static auto type_hash = ch(type_name);

		sElementRenderer() :
			System(type_name, type_hash)
		{
		}

		virtual void release() = 0;

		virtual void mark_dirty() = 0;

		FLAME_UNIVERSE_EXPORTS static sElementRenderer* create();
	};
}
