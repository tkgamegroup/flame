#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	namespace graphics
	{
		struct Device;

		struct Buffer
		{
			uint size;

			void *mapped;

			FLAME_GRAPHICS_EXPORTS void map(uint offset = 0, uint _size = 0);
			FLAME_GRAPHICS_EXPORTS void unmap();
			FLAME_GRAPHICS_EXPORTS void flush();

			FLAME_GRAPHICS_EXPORTS void copy_from_data(void *data);

			FLAME_GRAPHICS_EXPORTS static Buffer *create(Device *d, uint size, BufferUsage$ usage, MemProp$ mem_prop, bool sharing = false, void *data = nullptr);
			FLAME_GRAPHICS_EXPORTS static void destroy(Buffer *b);

		};
	}
}

