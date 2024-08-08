#pragma once

#include "window.h"
#include "buffer_private.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate : Swapchain
		{
#if USE_D3D12
			IDXGISwapChain3* d3d12_swapchain = nullptr;
#elif USE_VULKAN
			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;
#endif

			~SwapchainPrivate();

			int acquire_image() override;

			void build();
		};

		struct WindowPrivate : Window
		{
			WindowPrivate(NativeWindowPtr native);
			~WindowPrivate();
		};

		extern std::vector<WindowPtr> windows;
	}
}
