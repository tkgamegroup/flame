#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"
#include "systems/input.h"
#include "systems/scene.h"
#include "systems/renderer.h"
#include "systems/audio.h"

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;
	sInputPtr		input;
	sScenePtr		scene;
	sRendererPtr	renderer;
	sAudioPtr		audio;

	void create(std::string_view title, const uvec2& size = uvec2(1280, 720), 
		WindowStyleFlags styles = WindowFrame | WindowResizable,
		bool use_gui = false,
		bool graphics_debug = false, const std::vector<std::pair<uint, uint>>& graphics_configs = {})
	{
		GraphicsApplication::create(title, size, styles, use_gui, graphics_debug, graphics_configs);

		world.reset(World::create());
		input	 = (sInputPtr)world->add_system<sInput>();
		scene	 = (sScenePtr)world->add_system<sScene>();
		renderer = (sRendererPtr)world->add_system<sRenderer>();
		audio	 = (sAudioPtr)world->add_system<sAudio>();
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
