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
			DevicePrivate* _d;
#if defined(FLAME_VULKAN)
			VkSemaphore _v;
#endif

			SemaphorePrivate(Device* d);
			~SemaphorePrivate();

			void release() override { delete this; }
		};

		struct FencePrivate : Fence
		{
			DevicePrivate* _d;

			uint _vl;

#if defined(FLAME_VULKAN)
			VkFence _v;
#elif defined(FLAME_D3D12)
			ID3D12Fence* v;
			HANDLE ev;
#endif
			FencePrivate(Device* d);
			~FencePrivate();

			void _wait();

			void release() override { delete this; }

			void wait() override { _wait(); };
		};
	}
}
