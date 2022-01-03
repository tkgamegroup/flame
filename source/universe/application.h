#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"
#include "systems/node_renderer.h"

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;
	sNodeRendererPtr node_renderer;

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		GraphicsApplication::create(graphics_debug, title, size, styles);

		world.reset(World::create());
		//node_renderer = (sNodeRendererPtr)world->add_system(th<sNodeRenderer>());
	}

	bool on_update() override
	{
		world->update();
		GraphicsApplication::on_update();
		return true;
	}
};
