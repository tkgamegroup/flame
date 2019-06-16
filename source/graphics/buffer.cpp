#include "device_private.h"
#include "buffer_private.h"
#include "commandbuffer_private.h"

namespace flame
{
	namespace graphics
	{
		BufferPrivate::BufferPrivate(Device *_d, uint _size, BufferUsage$ _usage, MemProp$ _mem_prop, bool sharing)
		{
			size = _size;
			mapped = nullptr;

			usage = _usage;
			mem_prop = _mem_prop;
			d = (DevicePrivate*)_d;

#if defined(FLAME_VULKAN)
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = size;
			buffer_info.usage = to_flags(usage);
			buffer_info.sharingMode = sharing ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = sharing ? 2 : 0;
			uint queue_family_idx[] = {
				(uint)d->gq_idx,
				(uint)d->tq_idx
			};
			buffer_info.pQueueFamilyIndices = sharing ? queue_family_idx : nullptr;

			auto res = vkCreateBuffer(d->v, &buffer_info, nullptr, &v);
			assert(res == VK_SUCCESS);

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(d->v, v, &mem_requirements);

			assert(size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = d->find_memory_type(mem_requirements.memoryTypeBits, mem_prop);

			chk_res(vkAllocateMemory(d->v, &alloc_info, nullptr, &m));

			chk_res(vkBindBufferMemory(d->v, v, m, 0));
#elif defined(FLAME_D3D12)

#endif
		}

		BufferPrivate::~BufferPrivate()
		{
			if (mapped)
				unmap();

#if defined(FLAME_VULKAN)
			vkFreeMemory(d->v, m, nullptr);
			vkDestroyBuffer(d->v, v, nullptr);
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::map(uint offset, uint _size)
		{
			if (_size == 0)
				_size = size;
#if defined(FLAME_VULKAN)
			chk_res(vkMapMemory(d->v, m, offset, _size, 0, &mapped));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::unmap()
		{
			if (mapped)
			{
#if defined(FLAME_VULKAN)
				vkUnmapMemory(d->v, m);
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
			range.memory = m;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			chk_res(vkFlushMappedMemoryRanges(d->v, 1, &range));
#elif defined(FLAME_D3D12)

#endif
		}

		void BufferPrivate::copy_from_data(void *data)
		{
			auto stag_buf = Buffer::create(d, size, BufferUsageTransferSrc, MemPropHost);
			stag_buf->map();
			memcpy(stag_buf->mapped, data, size);
			stag_buf->flush();
			stag_buf->unmap();

			auto cb = Commandbuffer::create(d->gcp);
			cb->begin(true);
			cb->copy_buffer(stag_buf, this, 1, &BufferCopy(0, 0, size));
			cb->end();
			d->gq->submit(cb, nullptr, nullptr, nullptr);
			d->gq->wait_idle();
			Commandbuffer::destroy(cb);

			Buffer::destroy(stag_buf);
		}

		void Buffer::map(uint offset, uint _size)
		{
			((BufferPrivate*)this)->map(offset, _size);
		}

		void Buffer::unmap()
		{
			((BufferPrivate*)this)->unmap();
		}

		void Buffer::flush()
		{
			((BufferPrivate*)this)->flush();
		}

		void Buffer::copy_from_data(void *data)
		{
			((BufferPrivate*)this)->copy_from_data(data);
		}

		Buffer *Buffer::create(Device *d, uint size, BufferUsage$ usage, MemProp$ mem_prop, bool sharing, void *data)
		{
			auto b = new BufferPrivate(d, size, usage, mem_prop, sharing);

			if (data)
				b->copy_from_data(data);

			return b;
		}

		void Buffer::destroy(Buffer *b)
		{
			delete (BufferPrivate*)b;
		}
	}
}

