#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;

	void create(bool graphics_debug = true)
	{
		world.reset(World::create());
	}
};
