#pragma once

#include "graphics.h"
#include "swapchain.h"

#if USE_IMGUI
#include <imgui.h>
#if USE_IM_FILE_DIALOG
#include <ImFileDialog.h>
#endif
#endif

namespace flame
{
	namespace graphics
	{
		struct Window
		{
			NativeWindow* native;
			std::unique_ptr<SwapchainT> swapchain;

			std::unique_ptr<RenderpassT> renderpass_clear;
			std::unique_ptr<RenderpassT> renderpass_load;
			std::vector<std::unique_ptr<FramebufferT>> framebuffers;

			Listeners<void(void* ctx)> imgui_callbacks;

			Listeners<void(uint, CommandBufferPtr)> renders;

			bool dirty = false;

			virtual ~Window() {}

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
