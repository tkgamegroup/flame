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

namespace flame
{
	struct App;

	struct GraphicsWindow
	{
		App* app;

		NativeWindow* native_window;

		UniPtr<graphics::Swapchain> swapchain;
		int swapchain_img_idx = -1;
		std::vector<UniPtr<graphics::CommandBuffer>> cbs;
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

		UniPtr<World> world;
		sScene* s_scene = nullptr;
		sDispatcher* s_dispatcher = nullptr;
		sRenderer* s_renderer = nullptr;
		sPhysics* s_physics = nullptr;
		Entity* root = nullptr;

		void create(bool graphics_debug = true, bool always_render = false);
		void run();
	};

	GraphicsWindow::GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, NativeWindowStyleFlags styles, NativeWindow* parent) :
		app(app)
	{
		native_window = NativeWindow::create(title, size, styles, parent);
		native_window->add_destroy_listener([](Capture& c) {
			c.thiz<GraphicsWindow>()->native_window = nullptr;
		}, Capture().set_thiz(this));

		swapchain.reset(graphics::Swapchain::create(graphics::Device::get_default(), native_window));
		cbs.resize(swapchain->get_images_count());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i].reset(graphics::CommandBuffer::create(graphics::CommandPool::get(graphics::Device::get_default())));
		submit_fence.reset(graphics::Fence::create(graphics::Device::get_default()));
		render_finished.reset(graphics::Semaphore::create(graphics::Device::get_default()));


		if (app->windows.empty())
		{
			app->main_window = this;
			app->s_dispatcher->setup(native_window);
			app->s_scene->setup(native_window);
			app->s_renderer->setup(native_window);
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

	void App::create(bool graphics_debug, bool always_render = false)
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

		world.reset(World::create());
		s_scene = sScene::create();
		world->add_system(s_scene);
		s_dispatcher = sDispatcher::create();
		world->add_system(s_dispatcher);
		s_physics = sPhysics::create();
		world->add_system(s_physics);
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
			auto thiz = c.thiz<App>();
			if (thiz->windows.empty())
			{
				c._current = INVALID_POINTER;
				return;
			}

			auto main_window = thiz->windows.front().get();

			thiz->s_dispatcher->update();
			thiz->s_physics->update();
			thiz->s_scene->update();
			if (thiz->s_renderer->is_dirty())
				main_window->swapchain_img_idx = main_window->swapchain->acquire_image();;
			thiz->s_renderer->update();

			for (auto it = thiz->windows.begin(); it != thiz->windows.end();)
			{
				auto w = it->get();
				if (!w->native_window)
				{
					if (w == thiz->main_window)
						c._current = INVALID_POINTER;
					it = thiz->windows.erase(it);
				}
				else
				{
					w->submit_fence->wait();
					if (w->swapchain_img_idx >= 0)
					{
						auto cb = w->cbs[w->swapchain_img_idx].get();

						cb->begin();
						if (main_window == w)
							thiz->s_renderer->record(w->swapchain_img_idx, cb);
						cb->end();

						auto queue = graphics::Queue::get(graphics::Device::get_default());
						queue->submit(1, &cb, w->swapchain->get_image_avalible(), w->render_finished.get(), w->submit_fence.get());
						queue->present(w->swapchain.get(), w->render_finished.get());
						w->swapchain_img_idx = -1;
					}

					it++;
				}
			}
		}, Capture().set_thiz(this));
	}
}
