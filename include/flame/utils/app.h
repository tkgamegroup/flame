#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/command.h>
#include <flame/graphics/image.h>
#include <flame/graphics/font.h>
#include <flame/graphics/canvas.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/universe/world.h>
#include <flame/universe/entity.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/layout.h>
#include <flame/universe/systems/type_setting.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/element_renderer.h>
#include <flame/universe/res_map.h>

namespace flame
{
	struct App;

	struct GraphicsWindow
	{
		App* app;

		Window* window = nullptr;
		graphics::Swapchain* swapchain = nullptr;
		int swapchain_image_index = -1;
		std::vector<graphics::CommandBuffer*> swapchain_commandbuffers;
		graphics::Fence* submit_fence = nullptr;
		graphics::Semaphore* render_finished_semaphore = nullptr;

		graphics::Canvas* canvas = nullptr;

		World* world = nullptr;
		sTypeSetting* s_type_setting = nullptr;
		sEventDispatcher* s_event_dispatcher = nullptr;
		sElementRenderer* s_element_renderer = nullptr;
		Entity* root = nullptr;

		GraphicsWindow(App* app, bool has_world, bool has_canvas, const char* title, const Vec2u size, WindowStyleFlags styles, Window* p = nullptr, bool maximized = false);
		virtual ~GraphicsWindow();
		void set_canvas_target();
		void virtual on_frame() {}
		void update();
	};

	struct App
	{
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;

		graphics::Device* graphics_device;
		graphics::CommandPool* graphics_command_pool;
		graphics::Queue* graphics_queue;
		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas;

		std::list<std::unique_ptr<GraphicsWindow>> windows;
		GraphicsWindow* main_window = nullptr;

		void create(bool graphics_debug = true);
		void run();
	};

	GraphicsWindow::GraphicsWindow(App* app, bool has_world, bool has_canvas, const char* title, const Vec2u size, WindowStyleFlags styles, Window* p, bool maximized) :
		app(app)
	{
		if (maximized)
			styles = styles | WindowMaximized;
		window = Window::create(title, size, styles, p);
		window->add_destroy_listener([](Capture& c) {
			c.thiz<GraphicsWindow>()->window = nullptr;
		}, Capture().set_thiz(this));
		swapchain = graphics::Swapchain::create(app->graphics_device, window);
		swapchain_commandbuffers.resize(swapchain->get_images_count());
		for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
			swapchain_commandbuffers[i] = graphics::CommandBuffer::create(app->graphics_command_pool);
		submit_fence = graphics::Fence::create(app->graphics_device);
		render_finished_semaphore = graphics::Semaphore::create(app->graphics_device);

		if (has_world)
			has_canvas = true;
		if (has_canvas)
		{
			canvas = graphics::Canvas::create(app->graphics_device);
			set_canvas_target();
			canvas->set_resource(-1, app->font_atlas->get_view(), app->graphics_device->get_sampler(graphics::FilterNearest), "", nullptr, app->font_atlas);

			window->add_resize_listener([](Capture& c, const Vec2u&) {
				c.thiz<GraphicsWindow>()->set_canvas_target();
			}, Capture().set_thiz(this));
		}
		if (has_world)
		{
			world = World::create();
			world->register_object(window, "flame::Window");
			world->register_object(canvas, "flame::graphics::Canvas");
			s_type_setting = sTypeSetting::create();
			world->add_system(s_type_setting);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_element_renderer = sElementRenderer::create();
			struct sFrame : System
			{
				GraphicsWindow* thiz;

				sFrame(GraphicsWindow* _w) :
					System("sFrame", ch("sFrame"))
				{
					thiz = _w;
				}

				void update() override
				{
					if (!thiz->s_element_renderer->is_dirty())
						return;
					if (thiz->swapchain_image_index < 0)
					{
						if (!thiz->swapchain_commandbuffers.empty())
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
			world->add_system(new sFrame(this));
			world->add_system(s_element_renderer);

			root = world->get_root();
			root->add_component(cElement::create());
			{
				auto cer = cEventReceiver::create();
				cer->set_ignore_occluders(true);
				root->add_component(cer);
			}
			root->add_component(cLayout::create());
		}

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
			app->main_window = nullptr;
		if (window)
			window->close();
		swapchain->release();
		for (auto cb : swapchain_commandbuffers)
			cb->release();
		submit_fence->release();
		render_finished_semaphore->release();
		if (canvas)
			canvas->release();
		if (world)
			world->release();
	}

	void GraphicsWindow::set_canvas_target()
	{
		std::vector<graphics::ImageView*> vs(swapchain->get_images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = swapchain->get_image(i)->get_default_view();
		canvas->set_target(vs.size(), vs.data());
	}

	void GraphicsWindow::update()
	{
		if (world)
			world->update();

		submit_fence->wait();
		if (swapchain_image_index >= 0)
		{
			auto cb = swapchain_commandbuffers[swapchain_image_index];

			if (canvas)
				canvas->record(cb, swapchain_image_index);

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
		},
			[](void* p, uint size) {
			auto& str = *(std::string*)p;
			str.resize(size);
			return str.data();
		},
			[](void* p, uint size) {
			auto& str = *(std::wstring*)p;
			str.resize(size);
			return str.data();
		});

		//{
		//	auto config = parse_ini_file(L"config.ini");
		//	for (auto& e : config.get_section_entries(""))
		//	{
		//		if (e.key == "resource_path")
		//			resource_path = e.value;
		//	}
		//}

		engine_path = getenv("FLAME_PATH");

		graphics_device = graphics::Device::create(graphics_debug);
		graphics_command_pool = graphics_device->get_command_pool(graphics::QueueGraphics);
		graphics_queue = graphics_device->get_queue(graphics::QueueGraphics);

		sound_device = sound::Device::create_player();
		sound_context = sound::Context::create(sound_device);
		sound_context->make_current();

		{
			graphics::Font* fonts[] = {
				graphics::Font::create(L"c:/windows/fonts/consola.ttf"),
				graphics::Font::create((engine_path / L"art/font_awesome.ttf").c_str())
			};
			font_atlas = graphics::FontAtlas::create(graphics_device, 2, fonts);
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
