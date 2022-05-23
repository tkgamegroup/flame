#pragma once

#include "swapchain.h"
#include "renderpass.h"

namespace flame
{
	namespace graphics
	{
		struct Window
		{
			NativeWindowPtr native;
			std::unique_ptr<SwapchainT> swapchain;

			Listeners<void(uint, CommandBufferPtr)> renderers;

			bool dirty = false;

			virtual ~Window() {}

			virtual void render() = 0;

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
	}
}
