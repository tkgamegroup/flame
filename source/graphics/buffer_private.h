#include "buffer.h"
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct BufferPrivate : Buffer
		{
			DevicePrivate* device;

			uint size;
			BufferUsageFlags usage;
			MemoryPropertyFlags mem_prop;

			void* mapped = nullptr;

			VkBuffer vk_buffer;
			VkDeviceMemory vk_memory;

			BufferPrivate(DevicePrivate* device, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop);
			~BufferPrivate();

			void create();
			void destroy();

			void release() override { delete this; }

			uint get_size() const override { return size; }

			void* get_mapped() const override { return mapped; }

			void* map(uint offset = 0, uint size = 0) override;
			void unmap() override;
			void flush() override;

			void recreate(uint new_size) override;
		};
	}
}

