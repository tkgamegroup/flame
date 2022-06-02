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

			void* resize_lis = nullptr;
			void* destroy_lis = nullptr;

			~SwapchainPrivate();

			int acquire_image() override;

			void build();
		};

		struct WindowPrivate : Window
		{
			std::unique_ptr<CommandBufferPrivate> commandbuffer;
			std::unique_ptr<FencePrivate> finished_fence;
			std::unique_ptr<SemaphorePrivate> finished_semaphore;

			void* mouse_lis = nullptr;
			void* mousemove_lis = nullptr;
			void* scroll_lis = nullptr;
			void* key_lis = nullptr;
			void* char_lis = nullptr;
			void* resize_lis = nullptr;
			void* destroy_lis = nullptr;

			WindowPrivate(NativeWindowPtr native);
			~WindowPrivate();

			void render() override;
		};

		extern std::vector<WindowPtr> windows;
	}
}
