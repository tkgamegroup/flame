#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/network/network.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/command.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/sound/device.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/physics/device.h>
#include <flame/physics/scene.h>
#include <flame/script/script.h>
#include <flame/universe/driver.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/receiver.h>
#include <flame/universe/systems/layout.h>
#include <flame/universe/systems/dispatcher.h>
#include <flame/universe/systems/physics.h>
#include <flame/universe/systems/renderer.h>
#include <flame/universe/world.h>

namespace flame
{
	struct App;

	struct GraphicsWindow
	{
		App* app;

		FlmPtr<Window> window;

		FlmPtr<graphics::Swapchain> swapchain;
		int swapchain_image_index = -1;
		std::vector<FlmPtr<graphics::CommandBuffer>> commandbuffers;
		FlmPtr<graphics::Fence> submit_fence;
		FlmPtr<graphics::Semaphore> render_finished;
		FlmPtr<graphics::RenderPreferences> render_preferences;
		FlmPtr<graphics::Canvas> canvas;

		FlmPtr<physics::Scene> physics_scene;

		FlmPtr<World> world;
		sLayout* s_layout = nullptr;
		sDispatcher* s_dispatcher = nullptr;
		sRenderer* s_renderer = nullptr;
		sPhysics* s_physics = nullptr;
		Entity* element_root = nullptr;
		Entity* node_root = nullptr;

		GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, WindowStyleFlags styles, bool hdr = false, bool always_update = false, Window* parent = nullptr);
		virtual ~GraphicsWindow();
		void set_canvas_output();
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
		commandbuffers.resize(swapchain->get_images_count());
		for (auto i = 0; i < commandbuffers.size(); i++)
			commandbuffers[i].reset(graphics::CommandBuffer::create(graphics::CommandPool::get(graphics::Device::get_default())));
		submit_fence.reset(graphics::Fence::create(graphics::Device::get_default()));
		render_finished.reset(graphics::Semaphore::create(graphics::Device::get_default()));

		//render_preferences.reset(graphics::RenderPreferences::create(graphics::Device::get_default(), hdr));
		//canvas.reset(graphics::Canvas::create(render_preferences.get()));
		//set_canvas_output();

		physics_scene.reset(physics::Scene::create(physics::Device::get_default(), -9.81f, 2));

		window->add_resize_listener([](Capture& c, const uvec2&) {
			//c.thiz<GraphicsWindow>()->set_canvas_output();
		}, Capture().set_thiz(this));

		world.reset(World::create());
		world->register_object(window.get(), "flame::Window");
		world->register_object(swapchain.get(), "flame::graphics::Swapchain");
		world->register_object(canvas.get(), "flame::graphics::Canvas");
		world->register_object(physics_scene.get(), "flame::physics::Scene");
		s_layout = sLayout::create();
		world->add_system(s_layout);
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
					thiz->swapchain_image_index = thiz->swapchain->acquire_image();
			}
		}, Capture().set_thiz(this));

		element_root = world->get_element_root();
		node_root = world->get_node_root();
		s_dispatcher->set_next_focusing(element_root->get_component_t<cReceiver>());

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

	void GraphicsWindow::set_canvas_output()
	{
		//return;
		std::vector<graphics::ImageView*> vs(swapchain->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = swapchain->get_image(i)->get_view();
		canvas->set_output(vs.size(), vs.data());
	}

	void GraphicsWindow::update()
	{
		if (world)
			world->update();

		submit_fence->wait();
		if (swapchain_image_index >= 0)
		{
			auto cb = commandbuffers[swapchain_image_index].get();

			cb->begin();
			s_renderer->record_drawing_commands(swapchain_image_index, cb);
			cb->end();

			auto queue = graphics::Queue::get(graphics::Device::get_default());
			queue->submit(1, &cb, swapchain->get_image_avalible(), render_finished.get(), submit_fence.get());
			queue->present(swapchain.get(), render_finished.get());
			swapchain_image_index = -1;
		}
	}

	void App::create(bool graphics_debug)
	{
		set_allocator(
			[](uint size) {
				return malloc(size);
			},
			[](void* p) {
				free(p);
			},
			[](void* p, uint size) {
				return realloc(p, size);
			}
		);

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
