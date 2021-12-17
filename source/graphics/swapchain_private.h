#pragma once

#include "graphics_private.h"
#include "swapchain.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate : Swapchain
		{
			DevicePrivate* device;

			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;

			void* resize_listener;

			~SwapchainPrivate();

			int acquire_image() override;

			void build();
		};
	}
}
