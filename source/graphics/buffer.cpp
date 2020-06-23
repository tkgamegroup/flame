#include "device_private.h"
#include "buffer_private.h"
#include "commandbuffer_private.h"

namespace flame
{
	namespace graphics
	{
		BufferPrivate::BufferPrivate(DevicePrivate* d, uint size, BufferUsageFlags usage, MemPropFlags mem_prop, bool sharing, void* data) :
			_d(d)
		{
			_size = size;
			_mapped = nullptr;

#if defined(FLAME_VULKAN)
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = _size;
			buffer_info.usage = to_backend_flags<BufferUsage>(usage);
			buffer_info.sharingMode = sharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = sharing ? 2 : 0;
			uint queue_family_idx[] = {
				(uint)d->_gq_idx,
				(uint)d->_tq_idx
			};
			buffer_info.pQueueFamilyIndices = sharing ? queue_family_idx : nullptr;

			auto res = vkCreateBuffer(d->_v, &buffer_info, nullptr, &_v);
			assert(res == VK_SUCCESS);

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(d->_v, _v, &mem_requirements);

			assert(_size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = d->_find_memory_type(mem_requirements.memoryTypeBits, mem_prop);

			chk_res(vkAllocateMemory(d->_v, &alloc_info, nullptr, &_m));

			chk_res(vkBindBufferMemory(d->_v, _v, _m, 0));
#elif defined(FLAME_D3D12)

#endif
			if (data)
				copy_from_data(data);
		}

		BufferPrivate::~BufferPrivate()
		{
			if (_mapped)
				unmap();

#if defined(FLAME_VULKAN)
			vkFreeMemory(_d->_v, _m, nullptr);
			vkDestroyBuffer(_d->_v, _v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::_map(uint offset, uint size)
		{
			if (_mapped)
				return;
			if (size == 0)
				size = _size;
#if defined(FLAME_VULKAN)
			chk_res(vkMapMemory(_d->_v, _m, offset, size, 0, &_mapped));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::_unmap()
		{
			if (_mapped)
			{
#if defined(FLAME_VULKAN)
				vkUnmapMemory(_d->_v, _m);
				_mapped = nullptr;
#elif defined(FLAME_D3D12)

#endif
			}
		}

		void BufferPrivate::_flush()
		{
#if defined(FLAME_VULKAN)
			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = nullptr;
			range.memory = _m;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			chk_res(vkFlushMappedMemoryRanges(_d->_v, 1, &range));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::_copy_from_data(void* data)
		{
			auto stag_buf = std::make_unique<BufferPrivate>(_d, _size, BufferUsageTransferSrc, MemPropHost);

			stag_buf->_map();
			memcpy(stag_buf->_mapped, data, _size);
			stag_buf->_flush();
			stag_buf->_unmap();

			auto cb = std::make_unique<CommandbufferPrivate>(_d->_graphics_commandpool.get());
			cb->_begin(true);
			cb->_copy_buffer(stag_buf.get(), this, { &BufferCopy(0, 0, _size), 1 });
			cb->_end();
			auto q = _d->_graphics_queue.get();
			q->_submit(std::array{ cb.get() }, nullptr, nullptr, nullptr);
			q->_wait_idle();
		}

		Buffer* Buffer::create(Device* d, uint size, BufferUsageFlags usage, MemPropFlags mem_prop, bool sharing, void* data)
		{
			return new BufferPrivate((DevicePrivate*)d, size, usage, mem_prop, sharing, data);
		}
	}
}

