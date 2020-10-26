#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct Semaphore
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Semaphore *create(Device *d);
		};

		struct Fence
		{
			virtual void release() = 0;

			virtual void wait() = 0;

			FLAME_GRAPHICS_EXPORTS static Fence* create(Device* device);
		};
	}
}

