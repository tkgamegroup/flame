#include "device_private.h"
#include "synchronize_private.h"

namespace flame
{
	namespace graphics
	{
		SemaphorePrivate::SemaphorePrivate(DevicePrivate* d) :
			device(d)
		{
			VkSemaphoreCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			chk_res(vkCreateSemaphore(device->vk_device, &info, nullptr, &vk_semaphore));
		}

		SemaphorePrivate::~SemaphorePrivate()
		{
			vkDestroySemaphore(device->vk_device, vk_semaphore, nullptr);
		}

		Semaphore* Semaphore::create(Device* d)
		{
			return new SemaphorePrivate((DevicePrivate*)d);
		}

		FencePrivate::FencePrivate(DevicePrivate* d) :
			device(d)
		{
			VkFenceCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			chk_res(vkCreateFence(device->vk_device, &info, nullptr, &vk_fence));
			value = 1;
		}

		FencePrivate::~FencePrivate()
		{
			vkDestroyFence(device->vk_device, vk_fence, nullptr);
		}

		void FencePrivate::wait()
		{
			if (value > 0)
			{
				chk_res(vkWaitForFences(device->vk_device, 1, &vk_fence, true, UINT64_MAX));
				chk_res(vkResetFences(device->vk_device, 1, &vk_fence));
				value = 0;
			}
		}

		Fence* Fence::create(Device* d)
		{
			return new FencePrivate((DevicePrivate*)d);
		}
	}
}

