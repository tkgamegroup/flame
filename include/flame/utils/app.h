#pragma once

#include <flame/foundation/foundation.h>
#include <flame/foundation/type_info.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/font.h>
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
		SysWindow* w;
		graphics::Device* d;
		graphics::SwapchainResizable* scr;
		graphics::Fence* fence;
		std::vector<graphics::Commandbuffer*> cbs;
		Array<graphics::Commandbuffer*> sc_cbs;
		graphics::Semaphore* render_finished;

		graphics::FontAtlas* font_atlas_pixel;
		graphics::Canvas* canvas;

		Universe* u;
		Entity* root;
		cElement* c_element_root;

		void create(const char* title, const Vec2u size, uint styles, bool maximized = false)
		{
			w = SysWindow::create(title, size, styles);
			if (maximized)
				w->set_maximized(true);
			d = graphics::Device::create(true);
			scr = graphics::SwapchainResizable::create(d, w);
			fence = graphics::Fence::create(d);
			sc_cbs.resize(scr->sc()->image_count());
			for (auto i = 0; i < sc_cbs.s; i++)
				sc_cbs[i] = graphics::Commandbuffer::create(d->gcp);
			render_finished = graphics::Semaphore::create(d);
			TypeinfoDatabase::load(L"flame_foundation.dll", true, true);
			TypeinfoDatabase::load(L"flame_graphics.dll", true, true);
			TypeinfoDatabase::load(L"flame_universe.dll", true, true);

			wchar_t* fonts[] = {
				L"c:/windows/fonts/msyh.ttc",
				L"../art/font_awesome.ttf"
			};
			font_atlas_pixel = graphics::FontAtlas::create(d, graphics::FontDrawPixel, 2, fonts);

			u = Universe::create();
			u->add_object(w);

			auto w = World::create();
			u->add_world(w);
			w->add_system(sLayoutManagement::create());
			w->add_system(sEventDispatcher::create());
			w->add_system(s2DRenderer::create(L"../renderpath/canvas/bp", scr, FLAME_CHASH("SwapchainResizable"), &sc_cbs));
			canvas = w->get_system(s2DRenderer)->canvas;
			canvas->add_font(font_atlas_pixel);
			root = w->root();
			c_element_root = cElement::create();
			root->add_component(c_element_root);
		}

		virtual void on_frame()
		{
		}

		void run()
		{
			auto sc = scr->sc();

			if (sc)
				sc->acquire_image();

			fence->wait();
			looper().process_events();

			on_frame();

			c_element_root->set_size(Vec2f(w->size));
			u->update();

			if (sc)
			{
				cbs.push_back(sc_cbs[sc->image_index()]);
				d->gq->submit(cbs.size(), cbs.data(), sc->image_avalible(), render_finished, fence);
				d->gq->present(sc, render_finished);
			}
			cbs.clear();
		}
	};
}
