#pragma once

#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/timer_management.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/utils/ui.h>

namespace flame
{
	struct App
	{
		struct Window
		{
			App* app;

			SysWindow* sys_window;
			graphics::Swapchain* swapchain;
			int swapchain_image_index;
			std::vector<graphics::Commandbuffer*> swapchain_commandbuffers;
			graphics::Fence* submit_fence;
			graphics::Semaphore* render_finished_semaphore;

			Window(App* _app, const char* title, const Vec2u size, uint styles, SysWindow* p = nullptr, bool maximized = false)
			{
				app = _app;
				app->windows.emplace_back(this);

				sys_window = SysWindow::create(title, size, styles, p);
				if (maximized)
					sys_window->set_maximized(true);
				sys_window->destroy_listeners.add([](void* c) {
					auto thiz = *(Window**)c;
					thiz->sys_window = nullptr;
					return true;
				}, Mail::from_p(this));
				swapchain = graphics::Swapchain::create(app->graphics_device, sys_window);
				swapchain_image_index = -1;
				swapchain_commandbuffers.resize(swapchain->image_count());
				for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
					swapchain_commandbuffers[i] = graphics::Commandbuffer::create(graphics::Commandpool::get_default(graphics::QueueGraphics));
				submit_fence = graphics::Fence::create(app->graphics_device);
				render_finished_semaphore = graphics::Semaphore::create(app->graphics_device);
			}

			~Window()
			{
				if (!sys_window)
					return;
				graphics::Swapchain::destroy(swapchain);
				SysWindow::destroy(sys_window);
				for (auto i = 0; i < swapchain_commandbuffers.size(); i++)
					graphics::Commandbuffer::destroy(swapchain_commandbuffers[i]);
				graphics::Fence::destroy(submit_fence);
				graphics::Semaphore::destroy(render_finished_semaphore);
			}

			void prepare_swapchain()
			{
				if (swapchain_image_index < 0)
				{
					if (swapchain->image_count())
					{
						swapchain->acquire_image();
						swapchain_image_index = swapchain->image_index();
					}
				}
			}

			void render()
			{
				submit_fence->wait();
				if (swapchain->image_count() && swapchain_image_index >= 0)
				{
					auto queue = graphics::Queue::get_default(graphics::QueueGraphics);
					queue->submit(1, &swapchain_commandbuffers[swapchain_image_index], swapchain->image_avalible(), render_finished_semaphore, submit_fence);
					queue->present(swapchain, render_finished_semaphore);
					swapchain_image_index = -1;
				}
			}
		};

		bool developing;
		std::filesystem::path engine_path;
		std::filesystem::path resource_path;
		std::vector<std::filesystem::path> used_files[2];

		graphics::Device* graphics_device;
		std::vector<std::unique_ptr<Window>> windows;
		Window* main_window;
		graphics::Canvas* canvas;

		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas;

		World* world;
		sTimerManagement* s_timer_management;
		sLayoutManagement* s_layout_management;
		sEventDispatcher* s_event_dispatcher;
		s2DRenderer* s_2d_renderer;
		Entity* root;
		cElement* c_element_root;

		void create(const char* title, const Vec2u size, uint styles, bool graphics_debug, bool maximized = false)
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
				set_file_callback([](void* c, const wchar_t* _filename) {
					auto& app = **(App**)c;
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
				}, Mail::from_p(this));
				std::filesystem::path this_app = get_app_path(true).str();
				report_used_file((this_app.parent_path().replace_filename(L"{c}") / this_app.filename()).c_str());
			}

			TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
			TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
			TypeinfoDatabase::load(L"flame_universe.dll", true, true);

			graphics_device = graphics::Device::create(graphics_debug);

			main_window = new Window(this, title, size, styles, nullptr, maximized);
			main_window->sys_window->resize_listeners.add([](void* c, const Vec2u&) {
				(*(App**)c)->set_canvas_target();
				return true;
			}, Mail::from_p(this));
			main_window->sys_window->destroy_listeners.add([](void*) {
				exit(0);
				return true;
			}, Mail());

			sound_device = sound::Device::create_player();
			sound_context = sound::Context::create(sound_device);
			sound_context->make_current();

			canvas = graphics::Canvas::create(graphics_device);
			set_canvas_target();

			auto font_awesome_path = engine_path / L"art/font_awesome.ttf";
			const wchar_t* fonts[] = {
				L"c:/windows/fonts/msyh.ttc",
				font_awesome_path.c_str(),
			};
			font_atlas = graphics::FontAtlas::create(graphics_device, 2, fonts);
			canvas->add_font(font_atlas);

			get_style = utils::style;

			world = World::create();
			world->add_object(main_window->sys_window);
			world->add_object(font_atlas);
			s_timer_management = sTimerManagement::create();
			world->add_system(s_timer_management);
			s_layout_management = sLayoutManagement::create();
			world->add_system(s_layout_management);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_2d_renderer = s2DRenderer::create(canvas);
			s_2d_renderer->before_update_listeners.add([](void* c) {
				auto thiz = *(App**)c;
				if (thiz->s_2d_renderer->pending_update)
				{
					thiz->main_window->prepare_swapchain();
					thiz->canvas->prepare();
				}
				return true;
			}, Mail::from_p(this));
			world->add_system(s_2d_renderer);

			root = world->root();

			utils::set_current_entity(root);
			c_element_root = utils::c_element();
			utils::c_event_receiver();
			utils::c_layout();
			utils::set_current_root(root);
			utils::push_font_atlas(font_atlas);
		}

		void set_canvas_target()
		{
			auto sc = main_window->swapchain;
			std::vector<graphics::Imageview*> vs(sc->image_count());
			for (auto i = 0; i < vs.size(); i++)
				vs[i] = sc->image(i)->default_view();

			canvas->set_target(vs.size(), vs.data());
		}

		void run()
		{
			{
				uint dt = looper().delta_time * 1000;
				if (dt < 16)
					sleep(16 - dt);
			}

			looper().process_events();

			c_element_root->set_size(Vec2f(main_window->sys_window->size));
			world->update();
			if (main_window->swapchain_image_index >= 0)
				canvas->record(main_window->swapchain_commandbuffers[main_window->swapchain_image_index], main_window->swapchain_image_index);

			for (auto it = windows.begin(); it != windows.end();)
			{
				auto w = it->get();
				if (!w->sys_window)
					it = windows.erase(it);
				else
				{
					w->render();
					it++;
				}
			}
		}
	};
}
