#pragma once

#include <flame/foundation/foundation.h>
#include <flame/foundation/typeinfo.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
#include <flame/sound/device.h>
#include <flame/sound/context.h>
#include <flame/sound/buffer.h>
#include <flame/sound/source.h>
#include <flame/universe/world.h>
#include <flame/universe/systems/timer_management.h>
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>
#include <flame/universe/utils/ui.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct App
	{
		struct Window
		{
			SysWindow* w;
			graphics::Swapchain* sc;
			bool sc_used;
			Array<graphics::Commandbuffer*> cbs;
			graphics::Fence* fence;
			graphics::Semaphore* semaphore;

			Window(const char* title, const Vec2u size, uint styles, graphics::Device* d, SysWindow* p = nullptr)
			{
				w = SysWindow::create(title, size, styles, p);
				w->destroy_listeners.add([](void* c) {
					auto thiz = *(Window**)c;
					thiz->w = nullptr;
					return true;
				}, Mail::from_p(this));
				sc = graphics::Swapchain::create(d, w);
				sc_used = false;
				cbs.resize(sc->image_count());
				for (auto i = 0; i < cbs.s; i++)
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
				for (auto i = 0; i < cbs.s; i++)
					graphics::Commandbuffer::destroy(cbs[i]);
				graphics::Fence::destroy(fence);
				graphics::Semaphore::destroy(semaphore);
			}

			void prepare_sc()
			{
				if (!sc_used)
				{
					if (sc->image_count())
					{
						sc->acquire_image();
						sc_used = true;
					}
				}
			}

			void render(graphics::Device* d)
			{
				fence->wait();
				if (sc->image_count() && sc_used)
				{
					d->gq->submit(1, &cbs[sc->image_index()], sc->image_avalible(), semaphore, fence);
					d->gq->present(sc, semaphore);
					sc_used = false;
				}
			}
		};

		graphics::Device* graphics_device;
		std::vector<std::unique_ptr<Window>> windows;

		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas;

		World* world;
		sTimerManagement* s_timer_management;
		sLayoutManagement* s_layout_management;
		sEventDispatcher* s_event_dispatcher;
		s2DRenderer* s_2d_renderer;
		graphics::Canvas* canvas;
		Entity* root;
		cElement* c_element_root;

		void create(const char* title, const Vec2u size, uint styles, bool graphics_debug, const std::filesystem::path& engine_path, bool maximized = false)
		{
			TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
			TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
			TypeinfoDatabase::load(L"flame_universe.dll", true, true);

			graphics_device = graphics::Device::create(graphics_debug);

			auto main_window = new Window(title, size, styles, graphics_device);
			windows.emplace_back(main_window);
			if (maximized)
				main_window->w->set_maximized(true);
			main_window->w->destroy_listeners.add([](void*) {
				exit(0);
				return true;
			}, Mail());

			sound_device = sound::Device::create_player();
			sound_context = sound::Context::create(sound_device);
			sound_context->make_current();

			auto msyh_path = L"c:/windows/fonts/msyh.ttc";
			if (!std::filesystem::exists(msyh_path))
				msyh_path = L"c:/windows/fonts/msyh.ttf";
			auto font_awesome_path = engine_path / L"art/font_awesome.ttf";
			const wchar_t* fonts[] = {
				msyh_path,
				font_awesome_path.c_str(),
			};
			font_atlas = graphics::FontAtlas::create(graphics_device, 2, fonts);

			world = World::create();
			world->add_object(main_window->w);
			s_timer_management = sTimerManagement::create();
			world->add_system(s_timer_management);
			s_layout_management = sLayoutManagement::create();
			world->add_system(s_layout_management);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_2d_renderer = s2DRenderer::create((engine_path / L"renderpath/canvas/bp").c_str(), main_window->sc, FLAME_CHASH("Swapchain"), &main_window->cbs);
			s_2d_renderer->before_update_listeners.add([](void* c) {
				auto thiz = *(App**)c;
				if (thiz->s_2d_renderer->pending_update)
					thiz->windows[0]->prepare_sc();
				return true;
			}, Mail::from_p(this));
			world->add_system(s_2d_renderer);
			canvas = s_2d_renderer->canvas;
			canvas->add_font(font_atlas);

			root = world->root();
			c_element_root = cElement::create();
			root->add_component(c_element_root);
		}

		void run()
		{
			{
				uint dt = looper().delta_time * 1000;
				if (dt < 16)
					sleep(16 - dt);
			}

			looper().process_events();

			c_element_root->set_size(Vec2f(windows[0]->w->size));
			world->update();

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
