#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct Buffer
		{
			virtual void release() = 0;

			virtual uint get_size() const = 0;

			virtual void* get_mapped() const = 0;

			virtual void map(uint offset = 0, uint _size = 0) = 0;
			virtual void unmap() = 0;
			virtual void flush() = 0;

			virtual void recreate(uint new_size) = 0;

			FLAME_GRAPHICS_EXPORTS static Buffer *create(Device *d, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop);
		};
	}
}

