#pragma once

#include <flame/universe/universe.h>

namespace flame
{
	struct Entity;

	FLAME_UNIVERSE_EXPORTS Entity* create_topmost(Entity* e, bool close_when_clicked = true); // add a "topmost" entity with an cElement component, the cElement component will take e's cElement's size
	FLAME_UNIVERSE_EXPORTS void destroy_topmost(Entity* e); // find a "topmost" entity to e, take all children from "topmost" and then remove the "topmost"
}
