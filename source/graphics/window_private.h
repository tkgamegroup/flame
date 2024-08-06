#pragma once

#include "window.h"
#include "buffer_private.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate : Swapchain
		{
			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;
			IDXGISwapChain3* d3d12_swapchain = nullptr;

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
