#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"
#include "systems/input.h"
#include "systems/scene.h"
#include "systems/renderer.h"

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;
	sInputPtr		input;
	sScenePtr		scene;
	sRendererPtr	renderer;

	void create(bool graphics_debug, std::string_view title, const uvec2& size = uvec2(1280, 720), 
		WindowStyleFlags styles = WindowFrame | WindowResizable)
	{
		GraphicsApplication::create(graphics_debug, title, size, styles);

		world.reset(World::create());
		input	 = (sInputPtr)world->add_system(th<sInput>());
		scene	 = (sScenePtr)world->add_system(th<sScene>());
		renderer = (sRendererPtr)world->add_system(th<sRenderer>());
	}

	void on_render() override
	{
	}

	bool on_update() override
	{
		graphics::gui_frame();
		world->update();
		GraphicsApplication::on_update();
		return true;
	}
};
