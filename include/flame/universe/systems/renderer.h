#pragma once

#include <flame/universe/system.h>

namespace flame
{
	namespace graphics
	{
		struct CommandBuffer;
		struct Canvas;
	}

	struct sRenderer : System
	{
		inline static auto type_name = "flame::sRenderer";
		inline static auto type_hash = ch(type_name);

		sRenderer() :
			System(type_name, type_hash)
		{
		}

		virtual void set_always_update(bool a) = 0;
		virtual bool is_dirty() const = 0;
		virtual void mark_dirty() = 0;

		FLAME_UNIVERSE_EXPORTS static sRenderer* create();
	};
}
