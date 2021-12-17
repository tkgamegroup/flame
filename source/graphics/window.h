#pragma once

#include "graphics.h"

#if USE_IMGUI
#include <imgui.h>
#endif

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

			virtual void* add_imgui_callback(const std::function<void(void* ctx)>& callback) = 0;
			virtual void remove_imgui_callback(void* lis) = 0;

			virtual void* add_renderer(const std::function<void(uint, CommandBufferPtr)>& callback) = 0;
			virtual void remove_renderer(void* lis) = 0;

			virtual void imgui_new_frame() = 0;

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
