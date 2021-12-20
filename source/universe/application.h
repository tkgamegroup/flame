#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		GraphicsApplication::create(graphics_debug, title, size, styles);

		world.reset(World::create());
	}
};
