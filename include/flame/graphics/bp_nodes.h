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

#include <flame/foundation/foundation.h>
#include <flame/graphics/graphics.h>

namespace flame
{
	struct BP_GraphicsSwapchain : R
	{
		void* swapchain$i;

		void* swapchain$o;
		void* window$o;
		void* image1$o;
		void* image2$o;
		void* renderpass_clear$o;
		void* renderpass_dont_clear$o;
		void* framebuffer1$o;
		void* framebuffer2$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsSwapchain();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsClearvalues : R
	{
		void* clearvalues$i;
		void* renderpass$i;
		Array<Bvec4> colors$i;

		void* clearvalues$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsClearvalues();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCommandbuffer : R
	{
		void* commandbuffer$i;

		void* commandbuffer$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsCommandbuffer();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdBegin : R
	{
		void* commandbuffer1$i;
		void* commandbuffer2$i;

		void* commandbuffer1$o;
		void* commandbuffer2$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsCmdBegin();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdEnd : R
	{
		void* commandbuffer1$i;
		void* commandbuffer2$i;

		void* commandbuffer1$o;
		void* commandbuffer2$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsCmdEnd();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdBeginRenderpass : R
	{
		void* commandbuffer1$i;
		void* commandbuffer2$i;
		void* renderpass$i;
		void* clearvalues$i;
		void* framebuffer$i;

		void* commandbuffer1$o;
		void* commandbuffer2$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsCmdBeginRenderpass();
		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdEndRenderpass : R
	{
		void* commandbuffer1$i;
		void* commandbuffer2$i;

		void* commandbuffer1$o;
		void* commandbuffer2$o;

		FLAME_GRAPHICS_EXPORTS BP_GraphicsCmdEndRenderpass();
		FLAME_GRAPHICS_EXPORTS void update();
	};
}
