#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/command.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/event_receiver.h>
#include <flame/universe/components/layout.h>

namespace flame
{
	struct App;

	struct GraphicsWindow
	{
		Window* window;
		graphics::Swapchain* swapchain;
		int swapchain_image_index;
		std::vector<graphics::CommandBuffer*> swapchain_commandbuffers;
		graphics::Fence* submit_fence;
		graphics::Semaphore* render_finished_semaphore;

		graphics::Canvas* canvas;

		World* world;
		sLayoutManagement* s_layout_management;
		sEventDispatcher* s_event_dispatcher;
		sElementRenderer* s_2d_renderer;
		Entity* root;
		cElement* root_element;

		GraphicsWindow();
		GraphicsWindow(App* app, bool has_world, bool has_canvas, const char* title, const Vec2u size, uint styles, Window* p = nullptr, bool maximized = false);
		virtual ~GraphicsWindow();
		void setup_as_main_window();
		void set_canvas_target();
		void prepare_swapchain();
		void virtual on_update() {}
		void update();
	};

	struct App
	{
		bool developing;
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;
		std::vector<std::filesystem::path> used_files[2];

		graphics::Device* graphics_device;
		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas;

		std::list<std::unique_ptr<GraphicsWindow>> windows;

		void create(bool graphics_debug = true);
		void run();
	};

	GraphicsWindow::GraphicsWindow() :
		window(nullptr),
		swapchain(nullptr),
		swapchain_image_index(-1),
		submit_fence(nullptr),
		render_finished_semaphore(nullptr),
		canvas(nullptr),
		world(nullptr),
		s_layout_management(nullptr),
		s_event_dispatcher(nullptr),
		s_2d_renderer(nullptr),
		root(nullptr),
		root_element(nullptr)
	{
	}

	GraphicsWindow::GraphicsWindow(App* app, bool has_world, bool has_canvas, const char* title, const Vec2u size, uint styles, Window* p, bool maximized) :
		GraphicsWindow()
	{
		app->windows.emplace_back(this);

		auto graphics_device = app->graphics_device;
		auto font_atlas = app->font_atlas;

		window = Window::create(title, size, styles | (maximized ? WindowMaximized : 0), p);
		window->add_destroy_listener([](Capture& c) {
			c.thiz<GraphicsWindow>()->window = nullptr;
		}, Capture().set_thiz(this));
		swapchain = graphics::Swapchain::create(graphics_device, window);
		swapchain_commandbuffers.resize(swapchain->images_count());
		for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
			swapchain_commandbuffers[i] = graphics::CommandBuffer::create(graphics::Commandpool::get_default(graphics::QueueGraphics));
		submit_fence = graphics::Fence::create(graphics_device);
		render_finished_semaphore = graphics::Semaphore::create(graphics_device);

		if (has_world)
			has_canvas = true;
		if (has_canvas)
		{
			canvas = graphics::Canvas::create(graphics_device);
			set_canvas_target();
			canvas->add_font(font_atlas);

			window->add_resize_listener([](Capture& c, const Vec2u&) {
				c.thiz<GraphicsWindow>()->set_canvas_target();
			}, Capture().set_thiz(this));
		}
		if (has_world)
		{
			world = World::create();
			world->objects.push_back(window);
			world->objects.push_back(font_atlas);
			s_layout_management = sLayoutManagement::create();
			world->add_system(s_layout_management);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_2d_renderer = sElementRenderer::create(canvas);
			s_2d_renderer->before_update_listeners.add([](Capture& c) { // TODO: add a system before s_2d_renderer to do detect
				auto thiz = c.thiz<GraphicsWindow>();
				if (thiz->s_2d_renderer->pending_update)
					thiz->prepare_swapchain();
				return true;
			}, Capture().set_thiz(this));
			world->add_system(s_2d_renderer);

			root = world->root;
			root_element = cElement::create();
			root->add_component(root_element);
			root->add_component(cEventReceiver::create());
			root->add_component(cLayout::create(LayoutFree));
		}
	}

	GraphicsWindow::~GraphicsWindow()
	{
		if (window)
			window->release();
		graphics::Swapchain::destroy(swapchain);
		for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
			graphics::CommandBuffer::destroy(swapchain_commandbuffers[i]);
		graphics::Fence::destroy(submit_fence);
		graphics::Semaphore::destroy(render_finished_semaphore);
		if (canvas)
			graphics::Canvas::destroy(canvas);
		if (world)
			World::destroy(world);
	}

	void GraphicsWindow::setup_as_main_window()
	{
		window->add_destroy_listener([](Capture&) {
			exit(0);
		}, Capture());
	}

	void GraphicsWindow::set_canvas_target()
	{
		std::vector<graphics::ImageView*> vs(swapchain->images_count());
		for (auto i = 0; i < vs.size(); i++)
			vs[i] = swapchain->image(i)->default_view();
		canvas->set_target(vs.size(), vs.data());
	}

	void GraphicsWindow::prepare_swapchain()
	{
		if (swapchain_image_index < 0)
		{
			if (swapchain->images_count())
			{
				swapchain->acquire_image();
				swapchain_image_index = swapchain->image_index();
			}
			if (canvas)
				canvas->prepare();
		}
	}

	void GraphicsWindow::update()
	{
		if (world)
		{
			root_element->set_size(Vec2f(window->get_size()));
			world->update();
		}

		on_update();

		submit_fence->wait();
		if (swapchain_image_index >= 0)
		{
			auto cb = swapchain_commandbuffers[swapchain_image_index];

			if (canvas)
				canvas->record(cb, swapchain_image_index);

			auto queue = graphics::Queue::get_default(graphics::QueueGraphics);
			queue->submit(1, &cb, swapchain->image_avalible(), render_finished_semaphore, submit_fence);
			queue->present(swapchain, render_finished_semaphore);
			swapchain_image_index = -1;
		}
	}

	void App::create(bool graphics_debug)
	{
		developing = false;
		{
			auto config = parse_ini_file(L"config.ini");
			for (auto& e : config.get_section_entries(""))
			{
				if (e.key == "developing")
					developing = e.value != "0";
				else if (e.key == "resource_path")
					resource_path = e.value;
				else if (e.key == "engine_path")
				{
					if (e.value == "{e}")
						engine_path = getenv("FLAME_PATH");
					else
						engine_path = e.value;
				}
			}
		}
		set_engine_path(engine_path.c_str());
		if (developing)
		{
			set_file_callback([](Capture& c, const wchar_t* _filename) {
				auto& app = *c.thiz<App>();
				std::filesystem::path filename = _filename;
				auto i = -1;
				{
					auto p = filename.lexically_relative(app.resource_path);
					if (!p.empty() && p.c_str()[0] != L'.')
					{
						filename = p;
						i = 1;
					}
				}
				if (i == -1)
				{
					auto p = filename.lexically_relative(app.engine_path);
					if (!p.empty() && p.c_str()[0] != L'.')
					{
						filename = p;
						i = 0;
					}
				}
				if (i == -1)
					return;
				for (auto& p : app.used_files[i])
				{
					if (p == filename)
						return;
				}
				app.used_files[i].push_back(filename);
			}, Capture().set_thiz(this));
			wchar_t app_path[260];
			get_app_path(app_path, true);
			auto this_app = std::filesystem::path(app_path);
			report_used_file((this_app.parent_path().replace_filename(L"{c}") / this_app.filename()).c_str());
		}

		Library::load(L"flame_foundation.dll", true, true);
		Library::load(L"flame_graphics.dll", true, true);
		Library::load(L"flame_universe.dll", true, true);

		graphics_device = graphics::Device::create(graphics_debug);

		sound_device = sound::Device::create_player();
		sound_context = sound::Context::create(sound_device);
		sound_context->make_current();

		auto font_awesome_path = engine_path / L"art/font_awesome.ttf";
		const wchar_t* fonts[] = {
			L"c:/windows/fonts/msyh.ttc",
			font_awesome_path.c_str(),
		};
		font_atlas = graphics::FontAtlas::create(graphics_device, 2, fonts);
	}

	void App::run()
	{
		{
			uint dt = get_looper()->delta_time * 1000;
			if (dt < 16)
				std::this_thread::sleep_for(std::chrono::milliseconds(16 - dt));
		}

		get_looper()->process_events();

		for (auto it = windows.begin(); it != windows.end();)
		{
			auto w = it->get();
			if (!w->window)
				it = windows.erase(it);
			else
			{
				w->update();
				it++;
			}
		}
	}
}
