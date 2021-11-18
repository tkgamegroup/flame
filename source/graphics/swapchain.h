#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Swapchain
		{
			NativeWindow* window;

			std::vector<std::unique_ptr<ImageT>> images;
			std::unique_ptr<SemaphoreT> image_avalible;
			uint image_index;

			virtual ~Swapchain() {}

			virtual int acquire_image() = 0;

			FLAME_GRAPHICS_EXPORTS static Format format;
			FLAME_GRAPHICS_EXPORTS static SwapchainPtr create(DevicePtr device, NativeWindow* window);
		};
	}
}
