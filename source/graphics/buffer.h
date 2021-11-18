#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Buffer
		{
			uint size;
			BufferUsageFlags usage;
			MemoryPropertyFlags mem_prop;

			void* mapped = nullptr;

			virtual ~Buffer() {};

			virtual void map(uint offset = 0, uint _size = 0) = 0;
			virtual void unmap() = 0;
			virtual void flush() = 0;

			virtual void recreate(uint new_size) = 0;

			FLAME_GRAPHICS_EXPORTS static BufferPtr create(DevicePtr device, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop);
		};
	}
}

