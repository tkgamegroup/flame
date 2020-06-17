#include <flame/graphics/buffer.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct BufferPrivate : Buffer
		{
			DevicePrivate* d;

			uint size;

			void* mapped;

#if defined(FLAME_VULKAN)
			VkBuffer v;
			VkDeviceMemory m;
#elif defined(FLAME_D3D12)
			ID3D12Resource* v;
#endif

			BufferPrivate(DevicePrivate* d, uint size, BufferUsageFlags usage, MemPropFlags mem_prop, bool sharing = false, void* data = nullptr);
			~BufferPrivate();

			void release() override;

			uint get_size() const override;

			void* get_mapped() const override;

			void map(uint offset = 0, uint _size = 0) override;
			void unmap() override;
			void flush() override;

			void copy_from_data(void* data) override;
		};
	}
}

