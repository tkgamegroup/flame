#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	FLAME_UNIVERSE_EXPORTS Entity* get_topmost(Entity* e);
	FLAME_UNIVERSE_EXPORTS Entity* create_topmost(Entity* e, bool penetrable = false, bool close_when_clicked = true, bool no_mousemove_to_open_menu = true); // add a "topmost" entity with an cElement component, the cElement component will take e's cElement's size
	FLAME_UNIVERSE_EXPORTS void destroy_topmost(Entity* e); // take all children from "topmost" and then remove the "topmost"
}
