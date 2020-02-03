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
			SysWindow*w;

			DevicePrivate* d;
#if defined(FLAME_VULKAN)
			VkSurfaceKHR s;
			VkSwapchainKHR v;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<Image*> images;
			SemaphorePrivate* image_avalible;

			uint image_index;

			SwapchainPrivate(Device *d, SysWindow*w);
			~SwapchainPrivate();

			void acquire_image();
		};

		struct SwapchainResizablePrivate : SwapchainResizable
		{
			SysWindow* w;

			Device* d;
			Swapchain* sc;
			void* resize_listener;

			SwapchainResizablePrivate(Device* d, SysWindow* w);
			~SwapchainResizablePrivate();
		};
	}
}
