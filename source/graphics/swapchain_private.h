#pragma once

#include "swapchain.h"

namespace flame
{
	namespace graphics
	{
		struct SwapchainPrivate : Swapchain
		{
			DevicePrivate* device;

			NativeWindow* window;

			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;

			std::vector<std::unique_ptr<ImagePrivate>> images;
			std::unique_ptr<SemaphorePrivate> image_avalible;

			uint image_index;

			void* resize_listener;

			SwapchainPrivate(DevicePrivate *device, NativeWindow* window);
			~SwapchainPrivate();

			void release() override { delete this; }

			NativeWindow* get_window() const override { return window; }
			uint get_images_count() const override { return images.size(); }
			ImagePtr get_image(uint idx) const override { return images[idx].get(); }
			SemaphorePtr get_image_avalible() const override { return image_avalible.get(); }

			uint get_image_index() const override { return image_index; }
			int acquire_image() override;

			void update();
		};
	}
}
