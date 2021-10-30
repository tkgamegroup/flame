#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/command.h>
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
	struct App;

	struct GraphicsWindow
	{
		App* app;

		NativeWindow* native_window;

		UniPtr<graphics::Swapchain> swapchain;
		int swapchain_img_idx = -1;
		UniPtr<graphics::Fence> submit_fence;
		UniPtr<graphics::Semaphore> render_finished;

		GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, NativeWindowStyleFlags styles, NativeWindow* parent = nullptr);
		virtual ~GraphicsWindow();
	};

	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		std::list<std::unique_ptr<GraphicsWindow>> windows;
		GraphicsWindow* main_window = nullptr;
		UniPtr<graphics::CommandBuffer> commandbuffer;

		UniPtr<World> world;
		sDispatcher* s_dispatcher = nullptr;
		sPhysics* s_physics = nullptr;
		sScene* s_scene = nullptr;
		sRenderer* s_renderer = nullptr;
		sImgui* s_imgui = nullptr;
		Entity* root = nullptr;

		void create(bool graphics_debug = true, bool always_render = false);
		void run();
		void update();
	};

	GraphicsWindow::GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, NativeWindowStyleFlags styles, NativeWindow* parent) :
		app(app)
	{
		native_window = NativeWindow::create(title, size, styles, parent);
		native_window->add_destroy_listener([](Capture& c) {
			c.thiz<GraphicsWindow>()->native_window = nullptr;
		}, Capture().set_thiz(this));

		swapchain.reset(graphics::Swapchain::create(nullptr, native_window));
		submit_fence.reset(graphics::Fence::create(nullptr));
		render_finished.reset(graphics::Semaphore::create(nullptr));


		if (app->windows.empty())
		{
			app->main_window = this;
			app->s_dispatcher->setup(native_window);
			app->s_scene->setup(native_window);
			app->s_renderer->setup(swapchain.get());
		}
		app->windows.emplace_back(this);
	}

	GraphicsWindow::~GraphicsWindow()
	{
		if (app->main_window == this)
		{
			app->main_window = nullptr;
			return;
		}
	}

	void App::create(bool graphics_debug, bool always_render)
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

		commandbuffer.reset(graphics::CommandBuffer::create(nullptr));

		graphics::Device::create(graphics_debug);
		physics::Device::create();
		sound::Device::create();
		script::Instance::create();
		load_default_prefab_types();

		world.reset(World::create());
		s_dispatcher = sDispatcher::create();
		world->add_system(s_dispatcher);
		s_physics = sPhysics::create();
		world->add_system(s_physics);
		s_scene = sScene::create();
		world->add_system(s_scene);
		s_renderer = sRenderer::create();
		if (always_render)
			s_renderer->set_always_update(true);
		world->add_system(s_renderer);

		root = world->get_root();
		root->add_component(cElement::create());
		auto cer = cReceiver::create();
		cer->set_floating(true);
		root->add_component(cer);
		s_dispatcher->set_next_focusing((cReceiverPtr)cer);

		auto scr_ins = script::Instance::get_default();
		scr_ins->push_object();
		scr_ins->push_pointer(world.get());
		scr_ins->set_member_name("p");
		scr_ins->set_object_type("flame::World");
		scr_ins->set_global_name("world");
		scr_ins->excute_file(L"world_setup.lua");
	}

	void App::run()
	{
		flame::run([](Capture& c, float) {
			c.thiz<App>()->update();
		}, Capture().set_thiz(this));
	}

	void App::update()
	{
		auto main_window = windows.front().get();

		if (s_imgui)
			s_imgui->update();
		s_dispatcher->update();
		s_physics->update();
		s_scene->update();
		if (s_renderer && s_renderer->is_dirty() || s_imgui)
			main_window->swapchain_img_idx = main_window->swapchain->acquire_image();
		if (s_renderer)
			s_renderer->update();

		for (auto it = windows.begin(); it != windows.end();)
		{
			auto w = it->get();
			if (!w->native_window)
				it = windows.erase(it);
			else
			{
				if (w->swapchain_img_idx >= 0)
				{
					w->submit_fence->wait();

					commandbuffer->begin();
					if (main_window == w)
						s_renderer->record(w->swapchain_img_idx, commandbuffer.get());
					commandbuffer->end();

					auto queue = graphics::Queue::get(nullptr);
					queue->submit(1, &commandbuffer, w->swapchain->get_image_avalible(), w->render_finished.get(), w->submit_fence.get());
					queue->present(w->swapchain.get(), w->render_finished.get());
					w->swapchain_img_idx = -1;
				}

				it++;
			}
		}
	}
}
