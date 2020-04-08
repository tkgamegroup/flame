#include <flame/foundation/blueprint.h>
#include "device_private.h"
#include "buffer_private.h"
#include "commandbuffer_private.h"

#include <flame/reflect_macros.h>

namespace flame
{
	namespace graphics
	{
		BufferPrivate::BufferPrivate(Device* _d, uint _size, BufferUsageFlags usage, MemPropFlags mem_prop, bool sharing) :
			d((DevicePrivate*)_d)
		{
			size = _size;
			mapped = nullptr;

#if defined(FLAME_VULKAN)
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = size;
			buffer_info.usage = to_backend_flags<BufferUsage>(usage);
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
			if (mapped)
				return;
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

		void BufferPrivate::copy_from_data(void* data)
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
			d->gq->submit(1, &cb, nullptr, nullptr, nullptr);
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

		void Buffer::copy_from_data(void* data)
		{
			((BufferPrivate*)this)->copy_from_data(data);
		}

		Buffer* Buffer::create(Device* d, uint size, BufferUsageFlags usage, MemPropFlags mem_prop, bool sharing, void* data)
		{
			auto b = new BufferPrivate(d, size, usage, mem_prop, sharing);

			if (data)
				b->copy_from_data(data);

			return b;
		}

		void Buffer::destroy(Buffer* b)
		{
			delete (BufferPrivate*)b;
		}

		struct R(R_Buffer)
		{
			BP::Node* n;

			BASE0;
			RV(uint, size, i);
			RV(BufferUsage, usage, i, m);
			RV(MemProp, mem_prop, i, m);

			BASE1;
			RV(Buffer*, out, o);

			FLAME_GRAPHICS_EXPORTS RF(R_Buffer)()
			{
				mem_prop = MemPropDevice;
			}

			FLAME_GRAPHICS_EXPORTS void RF(update)(uint frame)
			{
				auto out_frame = out_s()->frame();
				if (size_s()->frame() > out_frame || usage_s()->frame() > out_frame || mem_prop_s()->frame() > out_frame)
				{
					if (out)
						Buffer::destroy(out);
					auto d = Device::default_one();
					auto ok = true;
					if (!d)
						ok = false;
					if (!(size > 0))
					{
						size_s()->set_fail_message("x and y of size must bigger than 0");
						ok = false;
					}
					if (usage == 0)
					{
						usage_s()->set_fail_message("usage cannot not be empty");
						ok = false;
					}
					if (mem_prop == 0)
					{
						usage_s()->set_fail_message("mem prop cannot not be empty");
						ok = false;
					}
					if (ok)
					{
						out = Buffer::create(d, size, usage, mem_prop);
						if (mem_prop == (MemPropHost | MemPropHostCoherent))
							out->map();
					}
					else
						out = nullptr;
					out_s()->set_frame(frame);
				}
			}

			FLAME_GRAPHICS_EXPORTS RF(~R_Buffer)()
			{
				if (out)
					Buffer::destroy(out);
			}
		};
	}
}

