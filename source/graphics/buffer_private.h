#include <flame/graphics/buffer.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct BufferPrivate : Buffer
		{
			BufferUsage$ usage;
			MemProp$ mem_prop;

			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkBuffer v;
			VkDeviceMemory m;
#elif defined(FLAME_D3D12)
			ID3D12Resource* v;
#endif

			BufferPrivate(Device *d, uint size, BufferUsage$ usage, MemProp$ mem_prop, bool sharing = false);
			~BufferPrivate();

			void map(uint offset = 0, uint _size = 0);
			void unmap();
			void flush();

			void copy_from_data(void *data);
		};
	}
}

