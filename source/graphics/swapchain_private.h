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
			Window* w;

			DevicePrivate* d;

			ImageUsageFlags extra_usages;

#if defined(FLAME_VULKAN)
			VkSurfaceKHR s;
			VkSwapchainKHR v;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<std::unique_ptr<ImagePrivate>> images;
			std::unique_ptr<SemaphorePrivate> image_avalible;

			uint image_index;

			uint hash;

			void* resize_listener;

			SwapchainPrivate(Device *d, Window* w, ImageUsageFlags extra_usages = 0);
			~SwapchainPrivate();

			void release() override;

			Window* get_window() const override;
			uint get_images_count() const override;
			Image* get_image(uint idx) const override;
			Semaphore* get_image_avalible() const override;

			uint get_image_index() const override;
			void acquire_image() override;

			uint get_hash() const override;

			void update();
		};
	}
}
