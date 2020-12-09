#pragma once

#include "../entity_private.h"
#include <flame/universe/components/custom_drawing.h>

namespace flame
{
	struct cCustomDrawingPrivate : cCustomDrawing
	{
		std::vector<std::unique_ptr<Closure<void(Capture&, graphics::Canvas*)>>> drawers;

		void* add_drawer(void (*callback)(Capture& c, graphics::Canvas* canvas), const Capture& capture) override;
		void remove_drawer(void* lis) override;
	};
}
