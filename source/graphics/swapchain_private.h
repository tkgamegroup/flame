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

			ImageUsageFlags extra_usages;

#if defined(FLAME_VULKAN)
			VkSurfaceKHR vk_surface;
			VkSwapchainKHR vk_swapchain;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<std::unique_ptr<ImagePrivate>> images;
			std::unique_ptr<SemaphorePrivate> image_avalible;

			uint image_index;

			void* resize_listener;

			SwapchainPrivate(DevicePrivate *d, Window* w, ImageUsageFlags extra_usages = 0);
			~SwapchainPrivate();

			void _acquire_image();

			void release() override { delete this; }

			Window* get_window() const override { return _w; }
			uint get_images_count() const override { return _images.size(); }
			Image* get_image(uint idx) const override { return _images[idx].get(); }
			Semaphore* get_image_avalible() const override { return (Semaphore*)_image_avalible.get(); }

			uint get_image_index() const override { return _image_index; }
			void acquire_image() override { _acquire_image(); }

			void update();
		};
	}
}
