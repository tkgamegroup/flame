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
	struct BP_GraphicsDevice : R
	{
		void* in$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsSwapchain : R
	{
		void* in$i;

		void* out$o;
		void* window$o;
		void* image1$o;
		void* image2$o;
		void* renderpass_clear$o;
		void* renderpass_dont_clear$o;
		void* framebuffer1$o;
		void* framebuffer2$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsClearvalues : R
	{
		void* in$i;
		void* renderpass$i;
		Array<Bvec4> colors$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCommandbuffer : R
	{
		void* in$i;
		void* device$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdBegin : R
	{
		void* in1$i;
		void* in2$i;

		void* out1$o;
		void* out2$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdEnd : R
	{
		void* in1$i;
		void* in2$i;

		void* out1$o;
		void* out2$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdBeginRenderpass : R
	{
		void* in1$i;
		void* in2$i;
		void* renderpass$i;
		void* framebuffer$i;
		void* clearvalues$i;

		void* out1$o;
		void* out2$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};

	struct BP_GraphicsCmdEndRenderpass : R
	{
		void* in1$i;
		void* in2$i;

		void* out1$o;
		void* out2$o;

		FLAME_GRAPHICS_EXPORTS void update();
	};
}
