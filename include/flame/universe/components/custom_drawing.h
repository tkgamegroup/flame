#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	struct cCustomDrawing : Component
	{
		inline static auto type_name = "flame::cCustomDrawing";
		inline static auto type_hash = ch(type_name);

		cCustomDrawing() :
			Component(type_name, type_hash)
		{
		}

		virtual void* add_drawer(void (*callback)(Capture& c, graphics::Canvas* canvas), const Capture& capture) = 0;
		virtual void remove_drawer(void* d) = 0;

		FLAME_UNIVERSE_EXPORTS static cCustomDrawing* create();
	};
}
