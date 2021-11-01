#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/command.h>
#include <flame/graphics/window.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/sound/device.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/physics/device.h>
#include <flame/physics/scene.h>
#include <flame/script/script.h>
#include <flame/universe/component.h>
#include <flame/universe/entity.h>
#include <flame/universe/system.h>
#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/receiver.h>
#include <flame/universe/systems/scene.h>
#include <flame/universe/systems/dispatcher.h>
#include <flame/universe/systems/physics.h>
#include <flame/universe/systems/renderer.h>
#include <flame/universe/systems/imgui.h>

namespace flame
{
	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		UniPtr<graphics::Swapchain> swapchain;
		UniPtr<graphics::CommandBuffer> commandbuffer;
		UniPtr<graphics::Fence> submit_fence;
		UniPtr<graphics::Semaphore> render_finished;

		UniPtr<World> world;
		sDispatcher* s_dispatcher = nullptr;
		sPhysics* s_physics = nullptr;
		sScene* s_scene = nullptr;
		sRenderer* s_renderer = nullptr;
		sImgui* s_imgui = nullptr;
		Entity* root = nullptr;

		graphics::Window* window = nullptr;

		void create(bool graphics_debug = true, bool always_render = false)
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

			graphics::Device::create(graphics_debug);
			physics::Device::create();
			sound::Device::create();
			script::Instance::create();
			load_default_prefab_types();

			commandbuffer.reset(graphics::CommandBuffer::create(nullptr));
			submit_fence.reset(graphics::Fence::create(nullptr));
			render_finished.reset(graphics::Semaphore::create(nullptr));

			world.reset(World::create());
			s_renderer = sRenderer::create();
			if (always_render)
				s_renderer->set_always_update(true);
			world->add_system(s_renderer);
			s_scene = sScene::create();
			world->add_system(s_scene);
			s_physics = sPhysics::create();
			world->add_system(s_physics);
			s_dispatcher = sDispatcher::create();
			world->add_system(s_dispatcher);

			root = world->get_root();

			auto scr_ins = script::Instance::get_default();
			scr_ins->push_object();
			scr_ins->push_pointer(world.get());
			scr_ins->set_member_name("p");
			scr_ins->set_object_type("flame::World");
			scr_ins->set_global_name("world");
			scr_ins->excute_file(L"world_setup.lua");
		}

		void set_main_window(graphics::Window* _window)
		{
			window = _window;

			auto native_window = window->get_native();

			s_dispatcher->setup(native_window);
			s_scene->setup(native_window);
			s_renderer->setup(window);
		}

		void update()
		{
			//if (s_imgui)
			//	s_imgui->update();
			s_dispatcher->update();
			s_physics->update();
			s_scene->update();
			if (s_renderer)
				s_renderer->update();

			window->update();
		}
	};
}
