#pragma once

#include "../graphics/application.h"
#include "../graphics/buffer.h"
#include "../graphics/font.h"
#include "entity.h"
#include "world.h"

struct UniverseApplication : GraphicsApplication
{
	std::filesystem::path engine_path;
	std::filesystem::path resource_path;

	std::unique_ptr<World> world;
	Entity* root = nullptr;

	void create(bool graphics_debug = true)
	{
		//{
		//	auto config = parse_ini_file(L"config.ini");
		//	for (auto& e : config.get_section_entries(""))
		//	{
		//		if (e.key == "resource_path")
		//			resource_path = e.value;
		//	}
		//}

		engine_path = getenv("FLAME_PATH");

		world.reset(World::create());
	}
};
