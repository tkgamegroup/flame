#pragma once

#include <flame/graphics/synchronize.h>
#include "graphics_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct SemaphorePrivate : Semaphore
		{
			DevicePrivate* device;
#if defined(FLAME_VULKAN)
			VkSemaphore vk_semaphore;
#endif

			SemaphorePrivate(DevicePrivate* d);
			~SemaphorePrivate();

			void release() override { delete this; }
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* device;

			uint value;

#if defined(FLAME_VULKAN)
			VkFence vk_fence;
#elif defined(FLAME_D3D12)
			ID3D12Fence* v;
			HANDLE ev;
#endif
			FencePrivate(DevicePrivate* d);
			~FencePrivate();

			void _wait();

			void release() override { delete this; }

			void wait() override { _wait(); };
		};
	}
}
