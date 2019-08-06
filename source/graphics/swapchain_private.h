#pragma once

#include <flame/graphics/swapchain.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct SemaphorePrivate;

		struct SwapchainPrivate : Swapchain
		{
			Window *w;

			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkSurfaceKHR s;
			VkSwapchainKHR v;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<void*> images;
			SemaphorePrivate* image_avalible;

			uint image_index;

			SwapchainPrivate(Device *d, Window *w);
			~SwapchainPrivate();

			void acquire_image();
		};

		struct SwapchainResizablePrivate : SwapchainResizable
		{
			Window* w;

			Device* d;
			Swapchain* sc;
			void* resize_listener;
			int sc_frame;

			SwapchainResizablePrivate(Device* d, Window* w);
			~SwapchainResizablePrivate();
		};
	}
}
