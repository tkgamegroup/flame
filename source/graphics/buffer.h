#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Buffer
		{
			virtual void release() = 0;

			virtual uint get_size() const = 0;

			virtual void* get_mapped() const = 0;

			virtual void* map(uint offset = 0, uint _size = 0) = 0;
			virtual void unmap() = 0;
			virtual void flush() = 0;

			virtual void recreate(uint new_size) = 0;

			FLAME_GRAPHICS_EXPORTS static Buffer* create(Device* device, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop);
		};

		struct StagingBuffer : UniPtr<Buffer>
		{
			void* mapped;

			StagingBuffer(Device* device, uint size, void* data = nullptr, BufferUsageFlags extra_usage = BufferUsageNone)
			{
				reset(Buffer::create(device, size, BufferUsageTransferSrc | extra_usage, MemoryPropertyHost | MemoryPropertyCoherent));
				mapped = p->map();
				if (data)
					memcpy(mapped, data, size);
			}
		};
	}
}

