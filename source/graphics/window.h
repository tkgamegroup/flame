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

			inline ImagePtr current_image() const
			{
				return images[image_index].get();
			}

			virtual int acquire_image() = 0;

			FLAME_GRAPHICS_API static Format format;

			struct Create
			{
				virtual SwapchainPtr operator()(NativeWindow* window) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;
		};

		struct Window
		{
			NativeWindowPtr native;
			std::unique_ptr<SwapchainT> swapchain;

			virtual ~Window() {}

			struct Create
			{
				virtual WindowPtr operator()(NativeWindowPtr native) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct GetList
			{
				virtual const std::vector<WindowPtr>& operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetList& get_list;
		};

		extern Listeners<void(int, CommandBufferPtr)> renderers;
	}
}
