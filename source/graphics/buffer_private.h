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

#if defined(FLAME_VULKAN)
			VkBuffer vk_buffer;
			VkDeviceMemory vk_memory;
#elif defined(FLAME_D3D12)
			ID3D12Resource* v;
#endif

			BufferPrivate(DevicePrivate* d, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop, bool sharing = false, void* data = nullptr);
			~BufferPrivate();

			void release() override { delete this; }

			uint get_size() const override { return size; }

			void* get_mapped() const override { return mapped; }

			void map(uint offset = 0, uint size = 0) override;
			void unmap() override;
			void flush() override;

			void copy_from_data(void* data) override;
		};
	}
}

