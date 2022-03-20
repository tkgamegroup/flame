#pragma once

#include "graphics_private.h"
#include "swapchain.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate : Swapchain
		{
			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;

			void* resize_lis = nullptr;
			void* destroy_lis = nullptr;

			~SwapchainPrivate();

			int acquire_image() override;

			void build();
		};
	}
}
