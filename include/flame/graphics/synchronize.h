#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct Semaphore
		{
			FLAME_GRAPHICS_EXPORTS static Semaphore *create(Device *d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Semaphore *s);
		};

		struct Fence
		{
			uint vl;

			FLAME_GRAPHICS_EXPORTS static Fence* create(Device* d);
			FLAME_GRAPHICS_EXPORTS static void destroy(Fence* s);

			FLAME_GRAPHICS_EXPORTS void wait();
		};
	}
}

