#pragma once

#include <flame/foundation/serialize.h>
#include <flame/foundation/window.h>
#include <flame/foundation/blueprint.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/canvas.h>
#include <flame/graphics/font.h>

namespace flame
{
	struct BasicApp;
	typedef BasicApp* BasicAppPtr;

	struct BasicApp
	{
		Application *app;
		Window *w;
		graphics::Device *d;
		graphics::Swapchain *sc;
		graphics::Semaphore *image_avalible;
		graphics::Semaphore *render_finished;
		std::pair<graphics::Fence*, int> fences[3];
		int frame;

		virtual void on_create() {};

		void create(const std::string& title, const Vec2u& res, int style)
		{
			app = Application::create();
			w = Window::create(app, title, res, style);
			d = graphics::Device::/*get_shared*/create(true);
			sc = graphics::Swapchain::create(d, w);
			image_avalible = graphics::Semaphore::create(d);
			render_finished = graphics::Semaphore::create(d);
			for (auto i = 0; i < FLAME_ARRAYSIZE(fences); i++)
			{
				fences[i].first = graphics::Fence::create(d);
				fences[i].second = 1;
			}
			frame = 0;

			on_create();
		}

		virtual void do_run() {};

		void run()
		{
			auto thiz = this;
			app->run(Function<void(void* c)>(
			[](void* c) {
				(*((BasicAppPtr*)c))->do_run();
			}, sizeof(void*), &thiz));
		}
	};
}
