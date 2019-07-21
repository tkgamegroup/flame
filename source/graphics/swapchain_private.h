#pragma once

#include <flame/foundation/window.h>
#include <flame/graphics/swapchain.h>
#include "synchronize_private.h"

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;

		struct SwapchainPrivate : Swapchain
		{
			Window *w;

			DevicePrivate *d;
#if defined(FLAME_VULKAN)
			VkSurfaceKHR s;
			VkSwapchainKHR v;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<std::pair<Image*, Fence*>> images;

			uint image_index;

			SemaphorePrivate* image_avalible;

			SwapchainPrivate(Device *d, Window *w);
			~SwapchainPrivate();

			void acquire_image();
		};

		typedef SwapchainPrivate* SwapchainPrivatePtr;
	}
}
