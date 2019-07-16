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
		Window *w;
		graphics::Device *d;
		graphics::Swapchain **psc;
		graphics::Semaphore *image_avalible;
		graphics::Semaphore *render_finished;
		graphics::Fence* fences[3];
		int frame;

		virtual void on_create() {};

		void create(const std::string& title, const Vec2u& res, int style)
		{
			w = Window::create(title, res, style);
			d = graphics::Device::/*get_shared*/create(true);
			image_avalible = graphics::Semaphore::create(d);
			render_finished = graphics::Semaphore::create(d);
			for (auto i = 0; i < FLAME_ARRAYSIZE(fences); i++)
				fences[i] = graphics::Fence::create(d);
			frame = 0;

			on_create();
		}

		virtual void do_run() {};

		void run()
		{
			auto thiz = this;
			app_run([](void* c) {
				(*((BasicAppPtr*)c))->do_run();
			}, new_mail(&thiz));
		}
	};
}
