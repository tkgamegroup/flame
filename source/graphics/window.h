#pragma once

#include "graphics.h"

namespace flame
{
	namespace graphics
	{
		struct Window
		{
			virtual void release() = 0;

			virtual NativeWindow* get_native() const = 0;
			virtual SwapchainPtr get_swapchain() const = 0;

			virtual void* add_renderer(void (*render)(Capture& c, uint img_idx, CommandBuffer* commandbuffer), const Capture& capture) = 0;
			virtual void remove_renderer(void* c) = 0;

			virtual void mark_dirty() = 0;

			virtual void update() = 0;

			virtual Window* get_next() const = 0;

			FLAME_GRAPHICS_EXPORTS static Window* create(Device* device, NativeWindow* native);
			FLAME_GRAPHICS_EXPORTS static Window* get_first();
		};
	}
}
