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
		graphics::SwapchainResizable* swapchain;
		std::vector<graphics::Commandbuffer*> graphics_cbs;
		Array<graphics::Commandbuffer*> graphics_sc_cbs;
		graphics::Fence* render_fence;
		graphics::Semaphore* render_semaphore;

		sound::Device* sound_device;
		sound::Context* sound_context;

		graphics::FontAtlas* font_atlas_pixel;

		Universe* universe;
		World* world;
		sLayoutManagement* s_layout_management;
		sEventDispatcher* s_event_dispatcher;
		s2DRenderer* s_2d_renderer;
		graphics::Canvas* canvas;
		Entity* root;
		cElement* c_element_root;

		void create(const char* title, const Vec2u size, uint styles, const std::filesystem::path& engine_path, bool maximized = false)
		{
			TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
			TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
			TypeinfoDatabase::load(L"flame_universe.dll", true, true);

			window = SysWindow::create(title, size, styles);
			if (maximized)
				window->set_maximized(true);

			graphics_device = graphics::Device::create(true);
			swapchain = graphics::SwapchainResizable::create(graphics_device, window);
			graphics_sc_cbs.resize(swapchain->sc()->image_count());
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

			universe = Universe::create();
			universe->add_object(window);

			world = World::create();
			universe->add_world(world);
			s_layout_management = sLayoutManagement::create();
			world->add_system(s_layout_management);
			s_event_dispatcher = sEventDispatcher::create();
			world->add_system(s_event_dispatcher);
			s_2d_renderer = s2DRenderer::create((engine_path / L"renderpath/canvas/bp").c_str(), swapchain, FLAME_CHASH("SwapchainResizable"), &graphics_sc_cbs);
			world->add_system(s_2d_renderer);
			canvas = s_2d_renderer->canvas;
			canvas->add_font(font_atlas_pixel);
			root = world->root();
			c_element_root = cElement::create();
			root->add_component(c_element_root);
		}

		virtual void on_frame()
		{
		}

		void run()
		{
			auto sc = swapchain->sc();

			if (sc)
				sc->acquire_image();

			render_fence->wait();
			looper().process_events();

			on_frame();

			c_element_root->set_size(Vec2f(window->size));
			universe->update();

			if (sc)
			{
				graphics_cbs.push_back(graphics_sc_cbs[sc->image_index()]);
				graphics_device->gq->submit(graphics_cbs.size(), graphics_cbs.data(), sc->image_avalible(), render_semaphore, render_fence);
				graphics_device->gq->present(sc, render_semaphore);
			}
			graphics_cbs.clear();
		}
	};
}
