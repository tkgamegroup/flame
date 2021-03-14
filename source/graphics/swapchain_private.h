#pragma once

#include <flame/graphics/swapchain.h>

namespace flame
{
	namespace graphics
	{
		struct DevicePrivate;
		struct SemaphorePrivate;
		struct ImagePrivate;

		struct SwapchainPrivate : Swapchain
		{
			Window* window;

			DevicePrivate* device;

			VkSurfaceKHR vk_surface = 0;
			VkSwapchainKHR vk_swapchain = 0;

			std::vector<std::unique_ptr<ImagePrivate>> images;
			std::unique_ptr<SemaphorePrivate> image_avalible;

			uint image_index;

			void* resize_listener;

			SwapchainPrivate(DevicePrivate *d, Window* w);
			~SwapchainPrivate();

			void release() override { delete this; }

			Window* get_window() const override { return window; }
			uint get_images_count() const override { return images.size(); }
			Image* get_image(uint idx) const override { return images[idx].get(); }
			Semaphore* get_image_avalible() const override { return (Semaphore*)image_avalible.get(); }

			uint get_image_index() const override { return image_index; }
			int acquire_image() override;

			void update();
		};
	}
}
