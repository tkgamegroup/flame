#pragma once

#include <flame/graphics/graphics.h>

namespace flame
{
	struct Window;
	struct BP;

	namespace graphics
	{
		struct Device;
		struct Image;
		struct Imageview;
		struct Semaphore;
		struct Commandbuffer;

		FLAME_GRAPHICS_EXPORTS Format$ get_swapchain_format();

		struct Swapchain
		{
			FLAME_GRAPHICS_EXPORTS Window* window() const;
			FLAME_GRAPHICS_EXPORTS uint image_count() const;
			FLAME_GRAPHICS_EXPORTS Image* const* images() const;
			FLAME_GRAPHICS_EXPORTS Semaphore* image_avalible() const;

			FLAME_GRAPHICS_EXPORTS uint image_index() const;
			FLAME_GRAPHICS_EXPORTS void acquire_image();

			FLAME_GRAPHICS_EXPORTS static Swapchain *create(Device *d, Window *w);
			FLAME_GRAPHICS_EXPORTS static void destroy(Swapchain *s);
		};

		struct SwapchainResizable
		{
			bool changed;

			FLAME_GRAPHICS_EXPORTS Swapchain* sc() const;

			FLAME_GRAPHICS_EXPORTS void link_bp(BP* bp, void* cbs);

			FLAME_GRAPHICS_EXPORTS static SwapchainResizable* create(Device* d, Window* w);
			FLAME_GRAPHICS_EXPORTS static void destroy(SwapchainResizable* s);
		};
	}
}

