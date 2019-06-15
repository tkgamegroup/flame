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
			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkSemaphore v;
#endif

			SemaphorePrivate(Device *d);
			~SemaphorePrivate();
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkFence v;
#elif defined(FLAME_D3D12)
			ID3D12Fence* v;
			HANDLE ev;
			uint vl;
#endif
			FencePrivate(Device* d);
			~FencePrivate();

			void wait();
		};
	}
}
