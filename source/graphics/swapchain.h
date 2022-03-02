#pragma once

#include "image.h"
#include "command.h"

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

			FLAME_GRAPHICS_API static Format format;

			struct Create
			{
				virtual SwapchainPtr operator()(DevicePtr device, NativeWindow* window) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};
	}
}
