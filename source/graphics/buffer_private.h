#include <flame/graphics/buffer.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct BufferPrivate : Buffer
		{
			DevicePrivate* device;

			uint size;

			void* mapped = nullptr;

			VkBuffer vk_buffer;
			VkDeviceMemory vk_memory;

			BufferPrivate(DevicePrivate* device, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop);
			~BufferPrivate();

			void release() override { delete this; }

			uint get_size() const override { return size; }

			void* get_mapped() const override { return mapped; }

			void map(uint offset = 0, uint size = 0) override;
			void unmap() override;
			void flush() override;
		};

		struct ImmediateStagingBuffer
		{
			std::unique_ptr<BufferPrivate> buf;

			ImmediateStagingBuffer(uint size, void* data);
		};
	}
}

