//MIT License
//
//Copyright (c) 2018 wjs
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

#include <flame/graphics/renderpass.h>
#include <flame/graphics/swapchain.h>
#include <flame/graphics/commandbuffer.h>
#include <flame/graphics/bp_nodes.h>

namespace flame
{
	BP_GraphicsSwapchain::BP_GraphicsSwapchain() :
		swapchain$i(nullptr),
		swapchain$o(nullptr),
		window$o(nullptr),
		image1$o(nullptr),
		image2$o(nullptr),
		renderpass_clear$o(nullptr),
		renderpass_dont_clear$o(nullptr),
		framebuffer1$o(nullptr),
		framebuffer2$o(nullptr)
	{
	}

	void BP_GraphicsSwapchain::update()
	{
		if (swapchain$i)
		{
			swapchain$o = swapchain$i;
			auto sc = (graphics::Swapchain*)swapchain$i;
			window$o = sc->window();
			image1$o = sc->get_image(0);
			image2$o = sc->get_image(1);
			renderpass_clear$o = sc->get_renderpass_clear();
			renderpass_dont_clear$o = sc->get_renderpass_dont_clear();
			framebuffer1$o = sc->get_framebuffer(0);
			framebuffer2$o = sc->get_framebuffer(1);
		}
	}

	BP_GraphicsClearvalues::BP_GraphicsClearvalues() :
		clearvalues$i(nullptr),
		clearvalues$o(nullptr)
	{
	}

	void BP_GraphicsClearvalues::update()
	{
		if (clearvalues$i)
			clearvalues$o = clearvalues$i;
		else
		{
			if (renderpass$i)
				clearvalues$o = graphics::ClearValues::create((graphics::Renderpass*)renderpass$i);
		}

		if (clearvalues$o)
		{
			for (auto i = 0; i < colors$i.size; i++)
			{
				auto cv = (graphics::ClearValues*)clearvalues$o;
				cv->set(i, colors$i[i]);
			}
		}
	}

	//BP_GraphicsCommandbuffer::BP_GraphicsCommandbuffer() :
	//	commandbuffer$i(nullptr),
	//	commandbuffer$o(nullptr)
	//{
	//}

	//void BP_GraphicsCommandbuffer::update()
	//{

	//}
}
