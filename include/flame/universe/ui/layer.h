#pragma once

#include <flame/math.h>
#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	namespace ui
	{
		FLAME_UNIVERSE_EXPORTS Entity* get_top_layer(Entity* parent);
		FLAME_UNIVERSE_EXPORTS Entity* add_layer(Entity* parent, bool penetrable = false, bool close_when_clicked = true, bool menu_move_to_open = true, const Vec4c& col = Vec4c(0), bool size_fit_parent = false);
		FLAME_UNIVERSE_EXPORTS void remove_top_layer(Entity* parent, bool take = true);
	}
}
