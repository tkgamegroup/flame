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
	struct BP_GraphicsDevice$
	{
		static const int flame_graphics$m;

		void* in$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void initialize$c();
	};

	struct BP_GraphicsSwapchain$
	{
		static const int flame_graphics$m;

		void* in$i;

		void* out$o;
		void* window$o;
		void* image1$o;
		void* image2$o;
		void* renderpass_clear$o;
		void* renderpass_dont_clear$o;
		void* framebuffer1$o;
		void* framebuffer2$o;

		FLAME_GRAPHICS_EXPORTS void initialize$c();
		FLAME_GRAPHICS_EXPORTS void update$c();
	};

	struct BP_GraphicsClearvalues$
	{
		static const int flame_graphics$m;

		void* in$i;
		void* renderpass$i;
		CommonData* colors$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void initialize$c();
		FLAME_GRAPHICS_EXPORTS void finish$c();
		FLAME_GRAPHICS_EXPORTS void update$c();
	};

	struct BP_GraphicsCommandbuffer$
	{
		static const int flame_graphics$m;

		void* in$i;
		void* device$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void initialize$c();
		FLAME_GRAPHICS_EXPORTS void finish$c();
	}; 

	struct BP_GraphicsCmdBegin$
	{
		static const int flame_graphics$m;

		void* cmd1$i;
		void* cmd2$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update$c();
	};

	struct BP_GraphicsCmdEnd$
	{
		static const int flame_graphics$m;

		void* in$i;
		void* cmd1$i;
		void* cmd2$i;

		FLAME_GRAPHICS_EXPORTS void update$c();
	};

	struct BP_GraphicsCmdBeginRenderpass$
	{
		static const int flame_graphics$m;

		void* in$i;
		void* cmd1$i;
		void* cmd2$i;

		void* renderpass$i;
		void* framebuffer1$i;
		void* framebuffer2$i;
		void* clearvalues$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update$c();
	};

	struct BP_GraphicsCmdEndRenderpass$
	{
		static const int flame_graphics$m;

		void* in$i;
		void* cmd1$i;
		void* cmd2$i;

		void* out$o;

		FLAME_GRAPHICS_EXPORTS void update$c();
	};
}
