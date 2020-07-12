#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"

namespace flame
{
	namespace graphics
	{
		BufferPrivate::BufferPrivate(DevicePrivate* d, uint _size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop, bool sharing, void* data) :
			device(d)
		{
			size = _size;

#if defined(FLAME_VULKAN)
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = _size;
			buffer_info.usage = to_backend_flags<BufferUsageFlags>(usage);
			buffer_info.sharingMode = sharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = sharing ? 2 : 0;
			uint queue_family_idx[] = {
				(uint)d->graphics_queue_index,
				(uint)d->transfer_queue_index
			};
			buffer_info.pQueueFamilyIndices = sharing ? queue_family_idx : nullptr;

			auto res = vkCreateBuffer(d->vk_device, &buffer_info, nullptr, &vk_buffer);
			assert(res == VK_SUCCESS);

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(d->vk_device, vk_buffer, &mem_requirements);

			assert(_size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = d->find_memory_type(mem_requirements.memoryTypeBits, mem_prop);

			chk_res(vkAllocateMemory(d->vk_device, &alloc_info, nullptr, &vk_memory));

			chk_res(vkBindBufferMemory(d->vk_device, vk_buffer, vk_memory, 0));
#elif defined(FLAME_D3D12)

#endif
			if (data)
				copy_from_data(data);
		}

		BufferPrivate::~BufferPrivate()
		{
			if (mapped)
				unmap();

#if defined(FLAME_VULKAN)
			vkFreeMemory(device->vk_device, vk_memory, nullptr);
			vkDestroyBuffer(device->vk_device, vk_buffer, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::map(uint offset, uint _size)
		{
			if (mapped)
				return;
			if (_size == 0)
				_size = size;
#if defined(FLAME_VULKAN)
			chk_res(vkMapMemory(device->vk_device, vk_memory, offset, _size, 0, &mapped));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::unmap()
		{
			if (mapped)
			{
#if defined(FLAME_VULKAN)
				vkUnmapMemory(device->vk_device, vk_memory);
				mapped = nullptr;
#elif defined(FLAME_D3D12)

#endif
			}
		}

		void BufferPrivate::flush()
		{
#if defined(FLAME_VULKAN)
			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = nullptr;
			range.memory = vk_memory;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			chk_res(vkFlushMappedMemoryRanges(device->vk_device, 1, &range));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::copy_from_data(void* data)
		{
			auto stag_buf = std::make_unique<BufferPrivate>(device, size, BufferUsageTransferSrc, MemoryPropertyHost);

			stag_buf->map();
			memcpy(stag_buf->mapped, data, size);
			stag_buf->flush();
			stag_buf->unmap();

			auto cb = std::make_unique<CommandBufferPrivate>(device->graphics_command_pool.get());
			cb->begin(true);
			cb->copy_buffer(stag_buf.get(), this, { &BufferCopy(0, 0, size), 1 });
			cb->end();
			auto q = device->graphics_queue.get();
			q->submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->wait_idle();
		}

		Buffer* Buffer::create(Device* d, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop, bool sharing, void* data)
		{
			return new BufferPrivate((DevicePrivate*)d, size, usage, mem_prop, sharing, data);
		}
	}
}

