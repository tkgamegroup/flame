#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Event
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Event* create(Device* device);
		};

		struct Semaphore
		{
			virtual void release() = 0;

			FLAME_GRAPHICS_EXPORTS static Semaphore* create(Device* device);
		};

		struct Fence
		{
			virtual void release() = 0;

			virtual void wait() = 0;

			FLAME_GRAPHICS_EXPORTS static Fence* create(Device* device);
		};
	}
}

