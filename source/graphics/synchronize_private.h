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
			VkSemaphore vk_semaphore;

			SemaphorePrivate(DevicePrivate* device);
			~SemaphorePrivate();

			void release() override { delete this; }
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* device;
			uint value;
			VkFence vk_fence;

			FencePrivate(DevicePrivate* device);
			~FencePrivate();

			void release() override { delete this; }

			void wait() override;
		};
	}
}
