//MIT License
//
//Copyright (c) 2019 wjs
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#include <flame/graphics/device.h>
#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/bp_nodes.h>

namespace flame
{
	void BP_GraphicsDevice$::initialize$c()
	{
		if (in$i)
			out$o = in$i;
		else
			out$o = nullptr;
	}

	BP_GraphicsDevice$ bp_graphics_device_unused;

	void BP_GraphicsSwapchain$::initialize$c()
	{
		if (in$i)
		{
			out$o = in$i;
			auto sc = (graphics::Swapchain*)out$o;
			window$o = sc->window();
			image_count$o = sc->image_count();
			if (image_count$o > 0)
			{
				images$o.count = image_count$o;
				images$o.v = new voidptr[image_count$o];
				for (auto i = 0; i < image_count$o; i++)
					images$o.v[i] = sc->image(i);
			}
			else
			{
				images$o.count = 0;
				images$o.v = nullptr;
			}
			renderpass_clear$o = sc->renderpass(true);
			renderpass_dont_clear$o = sc->renderpass(false);
			if (image_count$o > 0)
			{
				framebuffers$o.count = image_count$o;
				framebuffers$o.v = new voidptr[image_count$o];
				for (auto i = 0; i < image_count$o; i++)
					framebuffers$o.v[i] = sc->framebuffer(i);
			}
			else
			{
				framebuffers$o.count = 0;
				framebuffers$o.v = nullptr;
			}
		}
		else
			out$o = nullptr;
	}

	void BP_GraphicsSwapchain$::finish$c()
	{
		delete[]images$o.v;
		delete[]framebuffers$o.v;
	}

	BP_GraphicsSwapchain$ bp_graphics_swapchain_unused;

	void BP_GraphicsClearvalues$::initialize$c()
	{
		if (in$i)
			out$o = in$i;
		else
		{
			if (renderpass$i)
				out$o = graphics::Clearvalues::create((graphics::Renderpass*)renderpass$i);
		}
	}

	void BP_GraphicsClearvalues$::finish$c()
	{
		if (!in$i)
			graphics::Clearvalues::destroy((graphics::Clearvalues*)out$o);
	}

	void BP_GraphicsClearvalues$::update$c()
	{
		if (out$o)
		{
			auto cv = (graphics::Clearvalues*)out$o;
			auto count = cv->renderpass()->attachment_count();
			for (auto i = 0; i < count; i++)
				cv->set(i, colors$i[i].b4());
		}
	}

	BP_GraphicsClearvalues$ bp_graphics_clearvalues_unused;

	void BP_GraphicsCommandbuffers$::initialize$c()
	{
		if (count$i > 0 && device$i)
		{
			out$o.count = count$i;
			out$o.v = new voidptr[count$i];
			for (auto i = 0; i < count$i; i++)
			{
				out$o.v[i] = graphics::Commandbuffer::create(((graphics::Device*)device$i)->gcp);
#if defined(FLAME_D3D12)
				((graphics::Commandbuffer*)out$o.v[i])->end();
#endif
			}
		}
		else
		{
			out$o.count = 0;
			out$o.v = nullptr;
		}
	}

	void BP_GraphicsCommandbuffers$::finish$c()
	{
		for (auto i = 0; i < out$o.count; i++)
			graphics::Commandbuffer::destroy((graphics::Commandbuffer*)out$o.v[i]);
		delete[]out$o.v;
	}

	BP_GraphicsCommandbuffers$ bp_graphics_commandbuffers_unused;
}
