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
		struct sBeforeRender : System
		{
			GraphicsWindow* thiz;

			sBeforeRender(GraphicsWindow* _w) :
				System("sBeforeRender", ch("sBeforeRender"))
			{
				thiz = _w;
			}

			void update() override
			{
				if (!thiz->s_renderer->is_dirty())
					return;
				if (thiz->swapchain_image_index < 0)
				{
					if (thiz->swapchain->get_images_count() > 0)
					{
						thiz->swapchain->acquire_image();
						thiz->swapchain_image_index = thiz->swapchain->get_image_index();
					}
					thiz->canvas->prepare();
				}

				if (thiz->swapchain_image_index >= 0)
					thiz->on_frame();
			}
		};

		App* app;

		Window* window = nullptr;

		graphics::Swapchain* swapchain = nullptr;
		int swapchain_image_index = -1;
		std::vector<graphics::CommandBuffer*> swapchain_commandbuffers;
		graphics::Fence* submit_fence = nullptr;
		graphics::Semaphore* render_finished_semaphore = nullptr;
		graphics::RenderPreferences* render_preferences = nullptr;
		graphics::Canvas* canvas = nullptr;

		physics::Scene* physics_scene = nullptr;

		World* world = nullptr;
		sLayout* s_layout = nullptr;
		sDispatcher* s_dispatcher = nullptr;
		sRenderer* s_renderer = nullptr;
		sPhysics* s_physic_world = nullptr;
		Entity* root = nullptr;

		GraphicsWindow(App* app, const wchar_t* title, const uvec2 size, WindowStyleFlags styles, bool hdr = false, bool always_update = false, Window* parent = nullptr);
		virtual ~GraphicsWindow();
		void set_canvas_output();
		virtual void on_frame() {}
		void update();
	};

	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		graphics::CommandPool* graphics_command_pool;
		graphics::Queue* graphics_queue;

		graphics::FontAtlas* font_atlas;

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
			c.thiz<GraphicsWindow>()->window = nullptr;
		}, Capture().set_thiz(this));

		swapchain = graphics::Swapchain::create(graphics::Device::get_default(), window);
		swapchain_commandbuffers.resize(swapchain->get_images_count());
		for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
			swapchain_commandbuffers[i] = graphics::CommandBuffer::create(app->graphics_command_pool);
		submit_fence = graphics::Fence::create(graphics::Device::get_default());
		render_finished_semaphore = graphics::Semaphore::create(graphics::Device::get_default());

		render_preferences = graphics::RenderPreferences::create(graphics::Device::get_default(), hdr);
		canvas = graphics::Canvas::create(render_preferences);
		set_canvas_output();
		canvas->set_element_resource(-1, { nullptr, nullptr, app->font_atlas }, "default_font");

		physics_scene = physics::Scene::create(physics::Device::get_default(), -9.81f, 2);

		window->add_resize_listener([](Capture& c, const uvec2&) {
			c.thiz<GraphicsWindow>()->set_canvas_output();
		}, Capture().set_thiz(this));

		world = World::create();
		world->register_object(window, "flame::Window");
		world->register_object(canvas, "flame::graphics::Canvas");
		world->register_object(physics_scene, "flame::physics::Scene");
		s_layout = sLayout::create();
		world->add_system(s_layout);
		s_dispatcher = sDispatcher::create();
		world->add_system(s_dispatcher);
		s_physic_world = sPhysics::create();
		world->add_system(s_physic_world);
		world->add_system(new sBeforeRender(this));
		s_renderer = sRenderer::create();
		if (always_update)
			s_renderer->set_always_update(true);
		world->add_system(s_renderer);

		root = world->get_root();
		root->add_component(cElement::create());
		{
			auto cer = cReceiver::create();
			cer->set_ignore_occluders(true);
			root->add_component(cer);
			s_dispatcher->set_next_focusing(cer);
		}

		auto scr_ins = script::Instance::get_default();
		scr_ins->push_object();
		scr_ins->push_pointer(root);
		scr_ins->set_member_name("p");
		scr_ins->set_object_type("flame::Entity");
		scr_ins->set_global_name("root");

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
		if (window)
			window->close();
		swapchain->release();
		for (auto cb : swapchain_commandbuffers)
			cb->release();
		submit_fence->release();
		render_finished_semaphore->release();
		if (canvas)
			canvas->release();
		if (physics_scene)
			physics_scene->release();
		if (world)
			world->release();
	}

	void GraphicsWindow::set_canvas_output()
	{
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
			auto cb = swapchain_commandbuffers[swapchain_image_index];

			cb->begin(false);
			if (canvas)
				canvas->record(cb, swapchain_image_index);
			cb->end();

			app->graphics_queue->submit(1, &cb, swapchain->get_image_avalible(), render_finished_semaphore, submit_fence);
			app->graphics_queue->present(swapchain, render_finished_semaphore);
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
		graphics_command_pool = graphics::CommandPool::get(graphics::Device::get_default());
		graphics_queue = graphics::Queue::get(graphics::Device::get_default());
		physics::Device::set_default(physics::Device::create());
		sound::Device::set_default(sound::Device::create());
		script::Instance::set_default(script::Instance::create());
		Entity::initialize();

		{
			graphics::Font* fonts[] = {
				graphics::Font::create(L"c:/windows/fonts/msyh.ttc"),
				graphics::Font::create((engine_path / L"assets/font_awesome.ttf").c_str())
			};
			font_atlas = graphics::FontAtlas::create(graphics::Device::get_default(), 2, fonts);
		}
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
