#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"
#include "extension.h"

namespace flame
{
	namespace graphics
	{
		BufferPrivate::~BufferPrivate()
		{
			if (app_exiting) return;

			destroy();
		}

		void BufferPrivate::create()
		{
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = size;
			buffer_info.usage = to_backend_flags<BufferUsageFlags>(usage);
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = 0;
			buffer_info.pQueueFamilyIndices = nullptr;

			chk_res(vkCreateBuffer(device->vk_device, &buffer_info, nullptr, &vk_buffer));

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(device->vk_device, vk_buffer, &mem_requirements);

			assert(size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = device->find_memory_type(mem_requirements.memoryTypeBits, mem_prop);

			chk_res(vkAllocateMemory(device->vk_device, &alloc_info, nullptr, &vk_memory));
			chk_res(vkBindBufferMemory(device->vk_device, vk_buffer, vk_memory, 0));
			register_object(vk_buffer, "Buffer", this);
			register_object(vk_memory, "Buffer Memory", this);
		}

		void BufferPrivate::destroy()
		{
			if (mapped)
				unmap();

			vkFreeMemory(device->vk_device, vk_memory, nullptr);
			vkDestroyBuffer(device->vk_device, vk_buffer, nullptr);
			unregister_object(vk_buffer);
			unregister_object(vk_memory);
		}

		void BufferPrivate::map(uint offset, uint _size)
		{
			if (mapped)
				return;
			if (_size == 0)
				_size = size;
			chk_res(vkMapMemory(device->vk_device, vk_memory, offset, _size, 0, &mapped));
			return;
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

		void BufferPrivate::recreate(uint new_size)
		{
			destroy();
			size = new_size;
			create();
		}

		struct BufferCreate : Buffer::Create
		{
			BufferPtr operator()(uint size, BufferUsageFlags usage, MemoryPropertyFlags mem_prop) override
			{
				auto ret = new BufferPrivate;
				ret->size = size;
				ret->usage = usage;
				ret->mem_prop = mem_prop;
				ret->create();

				return ret;
			}
		}Buffer_create;
		Buffer::Create& Buffer::create = Buffer_create;
	}
}

