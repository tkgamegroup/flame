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
#include <flame/universe/systems/layout_management.h>
#include <flame/universe/systems/event_dispatcher.h>
#include <flame/universe/systems/2d_renderer.h>
#include <flame/universe/components/element.h>

#include "../renderpath/canvas/canvas.h"

namespace flame
{
	struct App
	{
		SysWindow* window;

		graphics::Device* graphics_device;
		graphics::Swapchain* swapchain;
		bool swapchain_used;
		std::vector<graphics::Commandbuffer*> graphics_cbs;
		Array<graphics::Commandbuffer*> graphics_sc_cbs;
		graphics::Fence* render_fence;
		graphics::Semaphore* render_semaphore;

		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas_pixel;

		World* world;
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

			window = SysWindow::create(title, size, styles);
			if (maximized)
				window->set_maximized(true);

			graphics_device = graphics::Device::create(graphics_debug);
			swapchain = graphics::Swapchain::create(graphics_device, window);
			swapchain_used = false;
			graphics_sc_cbs.resize(swapchain->image_count());
			for (auto i = 0; i < graphics_sc_cbs.s; i++)
				graphics_sc_cbs[i] = graphics::Commandbuffer::create(graphics_device->gcp);
			render_fence = graphics::Fence::create(graphics_device);
			render_semaphore = graphics::Semaphore::create(graphics_device);

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
			font_atlas_pixel = graphics::FontAtlas::create(graphics_device, graphics::FontDrawPixel, 2, fonts);

			world = World::create();
			world->add_object(window);
			s_layout_management = sLayoutManagement::create();
			world->add_system(s_layout_management);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_2d_renderer = s2DRenderer::create((engine_path / L"renderpath/canvas/bp").c_str(), swapchain, FLAME_CHASH("Swapchain"), &graphics_sc_cbs);
			s_2d_renderer->before_update_listeners.add([](void* c) {
				auto thiz = *(App**)c;
				if (thiz->s_2d_renderer->pending_update)
					thiz->prepare_swapchain();
				return true;
			}, new_mail_p(this));
			world->add_system(s_2d_renderer);
			canvas = s_2d_renderer->canvas;
			canvas->add_font(font_atlas_pixel);
			root = world->root();
			c_element_root = cElement::create();
			root->add_component(c_element_root);
		}

		void prepare_swapchain()
		{
			if (!swapchain_used)
			{
				if (swapchain->image_count())
				{
					swapchain->acquire_image();
					swapchain_used = true;
				}
			}
		}

		void run()
		{
			{
				uint dt = looper().delta_time * 1000;
				if (dt < 16)
					sleep(16 - dt);
			}

			render_fence->wait();
			looper().process_events();

			//if (!graphics_cbs.empty())
			//	prepare_swapchain();

			c_element_root->set_size(Vec2f(window->size));
			world->update();

			if (swapchain->image_count() && swapchain_used)
			{
				graphics_cbs.push_back(graphics_sc_cbs[swapchain->image_index()]);
				graphics_device->gq->submit(graphics_cbs.size(), graphics_cbs.data(), swapchain->image_avalible(), render_semaphore, render_fence);
				graphics_device->gq->present(swapchain, render_semaphore);
				swapchain_used = false;
			}
			graphics_cbs.clear();
		}
	};
}
