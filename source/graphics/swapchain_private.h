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
			Window* w;

			DevicePrivate* d;
			bool add_trans_dst_usage;
#if defined(FLAME_VULKAN)
			VkSurfaceKHR s;
			VkSwapchainKHR v;
#elif defined(FLAME_D3D12)
			IDXGISwapChain3* v;
#endif

			std::vector<Image*> images;
			SemaphorePrivate* image_avalible;

			uint hash;

			uint image_index;

			void* resize_listener;

			SwapchainPrivate(Device *d, Window* w, bool add_trans_dst_usage);
			~SwapchainPrivate();

			void update();

			void acquire_image();
		};
	}
}
