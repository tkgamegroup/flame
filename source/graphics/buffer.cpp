#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"

namespace flame
{
	namespace graphics
	{
		BufferPrivate::BufferPrivate(DevicePrivate* d, uint _size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop, bool sharing) :
			device(d)
		{
			size = _size;

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
		}

		BufferPrivate::~BufferPrivate()
		{
			if (mapped)
				unmap();

			vkFreeMemory(device->vk_device, vk_memory, nullptr);
			vkDestroyBuffer(device->vk_device, vk_buffer, nullptr);
		}

		void BufferPrivate::map(uint offset, uint _size)
		{
			if (mapped)
				return;
			if (_size == 0)
				_size = size;
			chk_res(vkMapMemory(device->vk_device, vk_memory, offset, _size, 0, &mapped));
		}

		void BufferPrivate::unmap()
		{
			if (mapped)
			{
				vkUnmapMemory(device->vk_device, vk_memory);
				mapped = nullptr;
			}
		}

		void BufferPrivate::flush()
		{
			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = nullptr;
			range.memory = vk_memory;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			chk_res(vkFlushMappedMemoryRanges(device->vk_device, 1, &range));
		}

		Buffer* Buffer::create(Device* d, uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop, bool sharing)
		{
			return new BufferPrivate((DevicePrivate*)d, size, usage, mem_prop, sharing);
		}

		ImmediateStagingBuffer::ImmediateStagingBuffer(DevicePrivate* d, uint size, void* data)
		{
			buf.reset(new BufferPrivate(d, size, BufferUsageTransferSrc, MemoryPropertyHost));
			buf->map();
			memcpy(buf->mapped, data, size);
			buf->flush();
		}
	}
}

