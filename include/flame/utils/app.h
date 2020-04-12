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
			SysWindow* w;
			graphics::Device* d;
			graphics::Swapchain* sc;
			int img_idx;
			bool sc_updated;
			std::vector<graphics::Commandbuffer*> cbs;
			graphics::Commandbuffer* curr_cb;
			graphics::Fence* fence;
			graphics::Semaphore* semaphore;

			Window(const char* title, const Vec2u size, uint styles, graphics::Device* _d, SysWindow* p = nullptr, bool maximized = false)
			{
				w = SysWindow::create(title, size, styles, p);
				if (maximized)
					w->set_maximized(true);
				w->destroy_listeners.add([](void* c) {
					auto thiz = *(Window**)c;
					thiz->w = nullptr;
					return true;
				}, Mail::from_p(this));
				d = _d;
				sc = graphics::Swapchain::create(d, w);
				sc_updated = false;
				cbs.resize(sc->image_count());
				for (auto i = 0; i < cbs.size(); i++)
					cbs[i] = graphics::Commandbuffer::create(d->gcp);
				fence = graphics::Fence::create(d);
				semaphore = graphics::Semaphore::create(d);
			}

			~Window()
			{
				if (!w)
					return;
				graphics::Swapchain::destroy(sc);
				SysWindow::destroy(w);
				for (auto i = 0; i < cbs.size(); i++)
					graphics::Commandbuffer::destroy(cbs[i]);
				graphics::Fence::destroy(fence);
				graphics::Semaphore::destroy(semaphore);
			}

			void update_sc()
			{
				if (!sc_updated)
				{
					if (sc->image_count())
					{
						sc->acquire_image();
						img_idx = sc->image_index();
						curr_cb = cbs[img_idx];
						sc_updated = true;
					}
				}
			}

			void render(graphics::Device* d)
			{
				fence->wait();
				if (sc->image_count() && sc_updated)
				{
					d->gq->submit(1, &curr_cb, sc->image_avalible(), semaphore, fence);
					d->gq->present(sc, semaphore);
					sc_updated = false;
				}
			}
		};

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

		void create(const char* title, const Vec2u size, uint styles, bool graphics_debug, const std::filesystem::path& engine_path, bool maximized = false)
		{
			TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
			TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
			TypeinfoDatabase::load(L"flame_universe.dll", true, true);

			graphics_device = graphics::Device::create(graphics_debug);

			main_window = new Window(title, size, styles, graphics_device, nullptr, maximized);
			windows.emplace_back(main_window);
			main_window->w->resize_listeners.add([](void* c, const Vec2u&) {
				(*(App**)c)->set_canvas_target();
				return true;
			}, Mail::from_p(this));
			main_window->w->destroy_listeners.add([](void*) {
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

			world = World::create();
			world->add_object(main_window->w);
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
					thiz->main_window->update_sc();
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
			auto sc = main_window->sc;
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

			c_element_root->set_size(Vec2f(main_window->w->size));
			world->update();
			if (main_window->sc_updated)
				canvas->record(main_window->curr_cb, main_window->img_idx);

			for (auto it = windows.begin(); it != windows.end();)
			{
				auto w = it->get();
				if (!w->w)
					it = windows.erase(it);
				else
				{
					w->render(graphics_device);
					it++;
				}
			}
		}
	};
}
