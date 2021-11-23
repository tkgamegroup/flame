#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Window
		{
			NativeWindow* native;
			std::unique_ptr<SwapchainT> swapchain;

			bool dirty = false;

			virtual ~Window() {}

			virtual void* add_renderer(const std::function<void(uint, CommandBufferPtr)>& callback) = 0;
			virtual void remove_renderer(void* lis) = 0;

			virtual void update() = 0;

			struct Create
			{
				virtual WindowPtr operator()(DevicePtr device, NativeWindow* native) = 0;
			};
			FLAME_GRAPHICS_EXPORTS static Create& create;
		};

		FLAME_GRAPHICS_EXPORTS const std::vector<WindowPtr> get_windows();
	}
}
