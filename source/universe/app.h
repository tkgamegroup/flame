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

		UniPtr<Window> window;

		UniPtr<graphics::Swapchain> swapchain;
		int swapchain_img_idx = -1;
		std::vector<UniPtr<graphics::CommandBuffer>> cbs;
		UniPtr<graphics::Fence> submit_fence;
		UniPtr<graphics::Semaphore> render_finished;

		UniPtr<World> world;
		sScene* s_scene = nullptr;
		sDispatcher* s_dispatcher = nullptr;
		sRenderer* s_renderer = nullptr;
		sPhysics* s_physics = nullptr;
		Entity* root = nullptr;

		GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, WindowStyleFlags styles, bool hdr = false, bool always_update = false, Window* parent = nullptr);
		virtual ~GraphicsWindow();
		void set_targets();
		void update();
	};

	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		std::list<std::unique_ptr<GraphicsWindow>> windows;
		GraphicsWindow* main_window = nullptr;

		void create(bool graphics_debug = true);
		void run();
	};

	GraphicsWindow::GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, WindowStyleFlags styles, bool hdr, bool always_update, Window* parent) :
		app(app)
	{
		window = Window::create(title, size, styles, parent);
		window->add_destroy_listener([](Capture& c) {
			c.thiz<GraphicsWindow>()->window.release();
		}, Capture().set_thiz(this));

		swapchain.reset(graphics::Swapchain::create(graphics::Device::get_default(), window.get()));
		window->add_resize_listener([](Capture& c, const uvec2& size) {
			c.thiz<GraphicsWindow>()->set_targets();
		}, Capture().set_thiz(this));
		cbs.resize(swapchain->get_images_count());
		for (auto i = 0; i < cbs.size(); i++)
			cbs[i].reset(graphics::CommandBuffer::create(graphics::CommandPool::get(graphics::Device::get_default())));
		submit_fence.reset(graphics::Fence::create(graphics::Device::get_default()));
		render_finished.reset(graphics::Semaphore::create(graphics::Device::get_default()));

		world.reset(World::create());
		world->register_object(window.get(), "flame::Window");
		s_scene = sScene::create();
		world->add_system(s_scene);
		s_dispatcher = sDispatcher::create();
		world->add_system(s_dispatcher);
		s_physics = sPhysics::create();
		world->add_system(s_physics);
		s_renderer = sRenderer::create();
		if (always_update)
			s_renderer->set_always_update(true);
		world->add_system(s_renderer);
		world->add_update_listener([](Capture& c, System* system, bool before) {
			auto thiz = c.thiz<GraphicsWindow>();
			if (before && system == thiz->s_renderer)
			{
				if (thiz->s_renderer->is_dirty())
					thiz->swapchain_img_idx = thiz->swapchain->acquire_image();
			}
		}, Capture().set_thiz(this));
		uint update_list[] = {
			0, 1, 2, 0, 3
		};
		world->set_update_list(countof(update_list), update_list);

		set_targets();

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

		if (app->windows.empty())
		{
			app->main_window = this;
			window->add_destroy_listener([](Capture&) {
				exit(0);
			}, Capture());
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

	void GraphicsWindow::set_targets()
	{
		std::vector<graphics::ImageView*> vs;
		vs.resize(swapchain->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = swapchain->get_image(i)->get_view();

		s_renderer->set_targets(vs.size(), vs.data());
	}

	void GraphicsWindow::update()
	{
		if (world)
			world->update();

		submit_fence->wait();
		if (swapchain_img_idx >= 0)
		{
			auto cb = cbs[swapchain_img_idx].get();

			cb->begin();
			s_renderer->record(swapchain_img_idx, cb);
			cb->end();

			auto queue = graphics::Queue::get(graphics::Device::get_default());
			queue->submit(1, &cb, swapchain->get_image_avalible(), render_finished.get(), submit_fence.get());
			queue->present(swapchain.get(), render_finished.get());
			swapchain_img_idx = -1;
		}
	}

	void App::create(bool graphics_debug)
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

		network::initialize();
		graphics::Device::set_default(graphics::Device::create(graphics_debug));
		physics::Device::set_default(physics::Device::create());
		sound::Device::set_default(sound::Device::create());
		script::Instance::set_default(script::Instance::create());
		Entity::initialize();
	}

	void App::run()
	{
		looper().loop([](Capture& c, float) {
			auto thiz = c.thiz<App>();

			{
				auto t = (0.02 - looper().get_delta_time());
				if (t > 0.f)
					std::this_thread::sleep_for(std::chrono::milliseconds(uint(t * 1000)));
			}

			for (auto it = thiz->windows.begin(); it != thiz->windows.end();)
			{
				auto w = it->get();
				if (!w->window)
					it = thiz->windows.erase(it);
				else
				{
					w->update();
					it++;
				}
			}
		}, Capture().set_thiz(this));
	}
}
