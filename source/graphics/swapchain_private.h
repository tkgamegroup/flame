#pragma once

#include <flame/foundation/window.h>
#include <flame/graphics/swapchain.h>
#include "graphics_private.h"

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

			std::vector<Image*> images;

			uint image_index;

			SwapchainPrivate(Device *d, Window *w);
			~SwapchainPrivate();

			void create();
			void destroy();
			void acquire_image(Semaphore *signal_semaphore);
		};

		typedef SwapchainPrivate* SwapchainPrivatePtr;
	}
}
