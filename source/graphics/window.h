#pragma once

#include "swapchain.h"
#include "renderpass.h"

#if USE_IMGUI
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
	#if USE_IM_FILE_DIALOG
	#include <ImFileDialog.h>
	#endif
	#if USE_IM_GUIZMO
	#include <ImGuizmo.h>
	#endif
#endif

namespace flame
{
	namespace graphics
	{
		struct Window
		{
			NativeWindowPtr native;
			std::unique_ptr<SwapchainT> swapchain;

			RenderpassPtr renderpass_clear;
			RenderpassPtr renderpass_load;
			std::vector<std::unique_ptr<FramebufferT>> framebuffers;

			Listeners<void()> imgui_callbacks;

			Listeners<void(uint, CommandBufferPtr)> renderers;

			bool dirty = false;

			virtual ~Window() {}

			virtual void* imgui_context() = 0;
			virtual void imgui_new_frame() = 0;

			virtual void update() = 0;

			struct Create
			{
				virtual WindowPtr operator()(DevicePtr device, NativeWindowPtr native) = 0;
			};
			FLAME_GRAPHICS_API static Create& create;

			struct GetList
			{
				virtual const std::vector<WindowPtr>& operator()() = 0;
			};
			FLAME_GRAPHICS_API static GetList& get_list;
		};
	}
}
