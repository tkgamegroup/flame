#include "device_private.h"
#include "buffer_private.h"
#include "command_private.h"
#include "shader_private.h"
#include "auxiliary.h"

namespace flame
{
	namespace graphics
	{
		std::vector<BufferPtr> buffers;

		BufferPrivate::BufferPrivate()
		{
			buffers.push_back(this);
		}

		BufferPrivate::~BufferPrivate()
		{
			if (app_exiting) return;

			std::erase_if(buffers, [&](const auto& i) {
				return i == this;
			});

			destroy();
		}

		void BufferPrivate::create()
		{
#if USE_D3D12
			D3D12_HEAP_PROPERTIES heap_properties = {};
			if (mem_prop & MemoryPropertyDevice)
				heap_properties.Type = D3D12_HEAP_TYPE_DEFAULT;
			else if (mem_prop & MemoryPropertyHost)
			{
				if ((usage & BufferUsageTransferSrc) || (usage & BufferUsageVertex) || (usage & BufferUsageIndex) || (usage & BufferUsageIndirect))
					heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
				else if (usage & BufferUsageTransferDst)
					heap_properties.Type = D3D12_HEAP_TYPE_READBACK;
			}
			heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heap_properties.CreationNodeMask = 1;
			heap_properties.VisibleNodeMask = 1;
			assert(heap_properties.Type);
			D3D12_RESOURCE_DESC resource_desc = {};
			resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resource_desc.Alignment = 0;
			resource_desc.Width = size;
			resource_desc.Height = 1;
			resource_desc.DepthOrArraySize = 1;
			resource_desc.MipLevels = 1;
			resource_desc.Format = DXGI_FORMAT_UNKNOWN;
			resource_desc.SampleDesc.Count = 1;
			resource_desc.SampleDesc.Quality = 0;
			resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
			auto state = D3D12_RESOURCE_STATE_GENERIC_READ;
			if (mem_prop & MemoryPropertyDevice)
			{
				if (usage & BufferUsageVertex)
					state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
				if (usage & BufferUsageIndex)
					state = D3D12_RESOURCE_STATE_INDEX_BUFFER;
				if (usage & BufferUsageUniform)
					state = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			}
			check_dx_result(device->d3d12_device->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE, &resource_desc, state, nullptr, IID_PPV_ARGS(&d3d12_resource)));
			register_object(d3d12_resource, "Buffer", this);
#elif USE_VULKAN
			VkBufferCreateInfo buffer_info;
			buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.flags = 0;
			buffer_info.pNext = nullptr;
			buffer_info.size = size;
			buffer_info.usage = to_vk_flags<BufferUsageFlags>(usage);
			buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			buffer_info.queueFamilyIndexCount = 0;
			buffer_info.pQueueFamilyIndices = nullptr;

			check_vk_result(vkCreateBuffer(device->vk_device, &buffer_info, nullptr, &vk_buffer));

			VkMemoryRequirements mem_requirements;
			vkGetBufferMemoryRequirements(device->vk_device, vk_buffer, &mem_requirements);

			assert(size <= mem_requirements.size);

			VkMemoryAllocateInfo alloc_info;
			alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.pNext = nullptr;
			alloc_info.allocationSize = mem_requirements.size;
			alloc_info.memoryTypeIndex = device->find_memory_type(mem_requirements.memoryTypeBits, mem_prop);

			check_vk_result(vkAllocateMemory(device->vk_device, &alloc_info, nullptr, &vk_memory));
			check_vk_result(vkBindBufferMemory(device->vk_device, vk_buffer, vk_memory, 0));

			register_object(vk_buffer, "Buffer", this);
			register_object(vk_memory, "Buffer Memory", this);
#endif
		}

		void BufferPrivate::destroy()
		{
			if (mapped)
				unmap();

#if USE_D3D12
			unregister_object(d3d12_resource);
#elif USE_VULKAN
			vkFreeMemory(device->vk_device, vk_memory, nullptr);
			vkDestroyBuffer(device->vk_device, vk_buffer, nullptr);
			unregister_object(vk_buffer);
			unregister_object(vk_memory);
#endif
		}

		void BufferPrivate::map(uint offset, uint _size)
		{
			if (mapped)
				return;
			if (_size == 0)
				_size = size;
#if USE_D3D12
			check_dx_result(d3d12_resource->Map(0, nullptr, &mapped));
#elif USE_VULKAN
			check_vk_result(vkMapMemory(device->vk_device, vk_memory, offset, _size, 0, &mapped));
#endif
			return;
		}

		void BufferPrivate::unmap()
		{
			if (mapped)
			{
#if USE_D3D12
				d3d12_resource->Unmap(0, nullptr);
#elif USE_VULKAN
				vkUnmapMemory(device->vk_device, vk_memory);
#endif
				mapped = nullptr;
			}
		}

		void BufferPrivate::flush()
		{
#if USE_D3D12

#elif USE_VULKAN
			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = nullptr;
			range.memory = vk_memory;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			check_vk_result(vkFlushMappedMemoryRanges(device->vk_device, 1, &range));
#endif
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

