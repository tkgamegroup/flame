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
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkSemaphore v;
#endif

			SemaphorePrivate(Device* d);
			~SemaphorePrivate();

			void release() override { delete this; }
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* d;

			uint vl;

#if defined(FLAME_VULKAN)
			VkFence v;
#elif defined(FLAME_D3D12)
			ID3D12Fence* v;
			HANDLE ev;
#endif

			FencePrivate(Device* d);
			~FencePrivate();

			void release() override { delete this; }

			void wait() override;
		};
	}
}
