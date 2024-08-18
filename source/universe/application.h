#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"
#include "systems/tween.h"
#include "systems/scene.h"
#include "systems/input.h"
#include "systems/renderer.h"
#include "systems/hud.h"
#include "systems/audio.h"
#include "systems/graveyard.h"

struct UniverseApplicationOptions : GraphicsApplicationOptions
{
	bool use_tween = true;
	bool use_scene = true;
	bool use_input = true;
	bool use_renderer = true;
	bool use_hud = true;
	bool use_audio = true;
	bool use_graveyard = true;
};

struct UniverseApplication : GraphicsApplication
{
	std::unique_ptr<World> world;
	sTweenPtr		tween = nullptr;
	sInputPtr		input = nullptr;
	sScenePtr		scene = nullptr;
	sRendererPtr	renderer = nullptr;
	sHudPtr			hud = nullptr;
	sAudioPtr		audio = nullptr;
	sGraveyardPtr	graveyard = nullptr;

	void create(std::string_view title, const uvec2& size = uvec2(1280, 720),
		WindowStyleFlags styles = WindowStyleFrame | WindowStyleResizable,
		const UniverseApplicationOptions& options = {})
	{
		GraphicsApplication::create(title, size, styles, options);

		world.reset(World::create());
		if (options.use_tween)
			tween = (sTweenPtr)world->add_system<sTween>();
		if (options.use_scene)
			scene = (sScenePtr)world->add_system<sScene>();
		if (options.use_input)
			input = (sInputPtr)world->add_system<sInput>();
		if (options.use_renderer)
			renderer = (sRendererPtr)world->add_system<sRenderer>();
		if (options.use_hud)
			hud = (sHudPtr)world->add_system<sHud>();
		if (options.use_audio)
			audio = (sAudioPtr)world->add_system<sAudio>();
		if (options.use_graveyard)
			graveyard = (sGraveyardPtr)world->add_system<sGraveyard>();

		((sHud*)hud)->bind_window(main_window);
	}

	virtual void on_render()
	{
		GraphicsApplication::on_render();
		((sRenderer*)renderer)->render(command_buffer.get());
		((sHud*)hud)->render(main_window->swapchain->image_index, command_buffer.get());
	}

	bool on_update() override
	{
		world->update();
		GraphicsApplication::on_update();
		return true;
	}
};
