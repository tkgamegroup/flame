// MIT License
// 
// Copyright (c) 2019 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/foundation/window.h>
#include <flame/graphics/device.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/synchronize.h>
#include <flame/graphics/commandbuffer.h>

using namespace flame;

graphics::Device* d;
graphics::Swapchain* sc;
graphics::Commandbuffer* cbs[2];
graphics::Fence* fences[2];
int frame;

int main(int argc, char** args)
{
	auto app = Application::create();
	auto w = Window::create(app, "DX Test", Ivec2(1280, 720), WindowFrame);

	d = graphics::Device::create(false);
	sc = graphics::Swapchain::create(d, w);
	auto cv = graphics::ClearValues::create(sc->get_renderpass_clear());
	cv->set(0, Bvec4(255, 128, 0, 255));

	for (auto i = 0; i < 2; i++)
	{
		auto cb = graphics::Commandbuffer::create(d->gcp);
		cb->begin();
		cb->begin_renderpass(sc->get_renderpass_clear(), sc->get_framebuffer(i), cv);
		cb->end_renderpass();
		cb->end();
		cbs[i] = cb;
		fences[i] = graphics::Fence::create(d);
	}

	frame = 0;

	app->run(Function<void(void* c)>(
		[](void* c) {
			auto idx = frame % 2;

			sc->acquire_image(nullptr);

			d->gq->submit(cbs[sc->get_avalible_image_index()], nullptr, nullptr, fences[idx]);

			fence[idx]->wait();
			d->gq->present(sc, nullptr);

			frame++;
		}));

	return 0;
}

